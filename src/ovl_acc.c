/*
** RetroWrite II - overlay 6: desk accessories
**   Startup Preferences, Load DOS 3.3 File, Communications, BASIC.SYSTEM
*/
#pragma code-name (push, "OVL6")
#pragma rodata-name (push, "OVL6")
#pragma data-name (push, "OVL6")
#pragma bss-name (push, "OVL6BSS")

#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <apple2enh.h>
#include "rw.h"

unsigned char __fastcall__ acc_rdblk(unsigned char unit, unsigned block, void *buf);
unsigned char __fastcall__ basic_go(const char *path);
extern unsigned char qstub[], qstub_len[], qpath_off[];

static char msg[80];

/* a simple choice row: label and options; returns x ranges in ox0/ox1 */
static unsigned ox0[8], ox1[8];
/* these boxes are too big for the screen save-under buffer, so the
   window is drawn again when they close */
static void unbox(void)
{
    redraw_all();
    refresh();
}

static void __fastcall__ choice_row(unsigned x, unsigned char y, const char *label,
                                    const char *const *opt, unsigned char n, unsigned char sel,
                                    unsigned char c0, unsigned char c1)
{
    unsigned char k;
    lb_clear(13);
    ui_text(x, 9, F_BOLD, label);
    x += 132;
    for (k = 0; k < n; ++k) {
        ox0[k] = x;
        ui_text(x + 4, 9, 0, opt[k]);
        ox1[k] = gx + 4;
        if (k == sel) lb_fill(x, gx + 4, 0, 12, LB_XOR);
        else frame_rect(x, gx + 4, 0, 12);
        x = gx + 12;
    }
    present(y, 13, c0, c1);
}

static unsigned char __fastcall__ choice_hit(unsigned char n)
{
    unsigned char k;
    for (k = 0; k < n; ++k) if (mx >= ox0[k] && mx < ox1[k]) return k;
    return 0xFF;
}

/* OK/Cancel style buttons at the bottom of a box */
static void __fastcall__ two_buttons(unsigned x, unsigned char y, const char *b0, const char *b1)
{
    unsigned char b;
    nbtn = 2;
    blabel[0] = b0; blabel[1] = b1;
    for (b = 0; b < 2; ++b) {
        bx0[b] = x - b * 92;
        bx1[b] = bx0[b] + 84;
        by[b] = y;
        draw_button(bx0[b], bx1[b], y, blabel[b], b == 0);
    }
}

static void __fastcall__ box_title(unsigned x, unsigned char y, const char *t, unsigned char c0, unsigned char c1)
{
    lb_clear(11);
    ui_text(x, 8, F_BOLD, t);
    present(y, 11, c0, c1);
}

static void __fastcall__ prog_file(const char *name)
{
    strcpy(tmppath, progdir);
    strcat(tmppath, name);
}

/* ---------------------------------------------------------------------- */
/* Startup Preferences (saved in RW2.PREFS)                               */

static const char *const yn[2] = { "Yes", "No" };
static const char *const fnt[3] = { "Sans", "Serif", "Mono" };
static const char *const szs[3] = { "9 Point", "18 Point", "27 Point" };

#define QC0 8
#define QC1 71
#define QX0 (QC0 * 7 + 14)
#define QY0 40

static void prefs_row(unsigned char r)
{
    unsigned char y = QY0 + 22 + r * 18;
    switch (r) {
    case 0: choice_row(QX0, y, "Show the ruler:", yn, 2, !ruler_on, QC0 + 1, QC1 - 1); break;
    case 1: choice_row(QX0, y, "New documents:", fnt, 3, def_fmt & F_FONT, QC0 + 1, QC1 - 1); break;
    case 2: choice_row(QX0, y, "Size:", szs, 3, (def_fmt & F_SIZE) >> 2, QC0 + 1, QC1 - 1); break;
    }
}

static void prefs_folder(void)
{
    lb_clear(13);
    ui_text(QX0, 9, F_BOLD, "Documents:");
    ui_text(QX0 + 132, 9, 0, browsedir[0] ? browsedir : "(the disks)");
    present(QY0 + 76, 13, QC0 + 1, QC1 - 1);
    lb_clear(11);
    ui_text(QX0 + 132, 8, F_ITAL, "The last folder used is remembered.");
    present(QY0 + 90, 11, QC0 + 1, QC1 - 1);
}

static void prefs(void)
{
    unsigned char b, r, k, ro = ruler_on, df = def_fmt;
    int fd;
    caret_hide();
    ptr_hide();
    draw_box(QC0, QC1, QY0, QY0 + 130);
    box_title(QX0, QY0 + 6, "Startup Preferences", QC0 + 1, QC1 - 1);
    for (r = 0; r < 3; ++r) prefs_row(r);
    prefs_folder();
    two_buttons(QX0 + 330, QY0 + 108, "Save", "Cancel");
    for (;;) {
        b = run_buttons(1);
        if (b < 2) break;
        if (b != 0xFD) continue;
        for (r = 0; r < 3; ++r)
            if (my >= QY0 + 22 + r * 18 && my < QY0 + 35 + r * 18) {
                prefs_row(r);           /* refresh the hit ranges of this row */
                k = choice_hit(r ? 3 : 2);
                if (k == 0xFF) break;
                if (r == 0) ruler_on = !k;
                if (r == 1) def_fmt = (def_fmt & ~F_FONT) | k;
                if (r == 2) def_fmt = (def_fmt & ~F_SIZE) | (k << 2);
                ptr_hide();
                prefs_row(r);
            }
    }
    ptr_hide();
    unbox();
    if (b == 1) { ruler_on = ro; def_fmt = df; return; }
    /* RW2.PREFS: 'R', 'P', 1, printer type, slot, quality, page numbers,
       ruler, new-document format, documents folder (64) */
    msg[0] = 'R'; msg[1] = 'P'; msg[2] = 1;
    msg[3] = prt_type; msg[4] = prt_slot; msg[5] = prt_quality; msg[6] = prt_pnum;
    msg[7] = ruler_on; msg[8] = def_fmt;
    prog_file("/RW2.PREFS");
    _filetype = PRODOS_T_BIN;
    _auxtype = 0;
    fd = open(tmppath, O_WRONLY | O_CREAT | O_TRUNC);
    if (fd < 0 || write(fd, msg, 9) != 9 || write(fd, browsedir, 64) != 64)
        show_msg("Could not save RW2.PREFS.");
    if (fd >= 0) close(fd);
    if (ro != ruler_on) redraw_all();
}

/* ---------------------------------------------------------------------- */
/* Load DOS 3.3 File: text files from a DOS 3.3 disk in a 5.25" drive     */

/* DOS 3.3 sector -> (ProDOS block within the track, which half) */
static const unsigned char sblk[16] = { 0, 7, 6, 6, 5, 5, 4, 4, 3, 3, 2, 2, 1, 1, 0, 7 };
static const unsigned char shalf[16] = { 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1 };
static unsigned char dunit;
#define sect ((unsigned char *)0x1B00)     /* line buffer: not drawn while reading */

static unsigned char rerr;
static unsigned char __fastcall__ read_sec(unsigned char t, unsigned char s)
{
    if (t > 34 || s > 15) return 0;
    if ((rerr = acc_rdblk(dunit, t * 8 + sblk[s], iob))) return 0;
    memcpy(sect, iob + (shalf[s] << 8), 256);
    return 1;
}

#define MAXDF 24
struct dfile { unsigned char t, s; char name[31]; };
/* the file list borrows the spelling checker's word pool ($0C00), which
   is then read again from USER.DICT the next time it is needed */
#define df ((struct dfile *)0x0C00)
extern unsigned char upool_loaded;

static void dos_load(void)
{
    unsigned char ct, cs, i, n = 0, k, sel = 0, t, s, done = 0;
    unsigned p, total = 0, e, j;
    unsigned char *tsl = (unsigned char *)0x1C00;
    unsigned char *q;
    nbtn = 0;
    k = alert("Load DOS 3.3 File: which drive holds\nthe DOS 3.3 disk?", "S6 D1", "S6 D2", "Cancel");
    if (k == 2) return;
    dunit = 0x60 | (k ? 0x80 : 0);
    pshape = 2;
    ptr_hide(); ptr_show();
    if (!read_sec(17, 0) || sect[3] != 3) {
        pshape = 0;
        show_msg("That disk is not a DOS 3.3 disk\n(or there is no disk in the drive).");
        return;
    }
    upool_loaded = 0;
    ct = sect[1]; cs = sect[2];
    while (ct && n < MAXDF && read_sec(ct, cs)) {
        for (e = 0; e < 7 && n < MAXDF; ++e) {
            q = sect + 0x0B + e * 35;
            if (!q[0] || q[0] == 0xFF || (q[2] & 0x7F) != 0) continue;     /* text files only */
            df[n].t = q[0];
            df[n].s = q[1];
            for (i = 0; i < 30; ++i) df[n].name[i] = q[3 + i] & 0x7F;
            for (i = 30; i && df[n].name[i - 1] == ' '; --i) ;
            df[n].name[i] = 0;
            ++n;
        }
        ct = sect[1]; cs = sect[2];
    }
    pshape = 0;
    if (!n) { show_msg("There are no text files on that disk."); return; }
    /* choose a file */
    caret_hide();
    ptr_hide();
    draw_box(14, 65, 30, 50 + n * 11 + 6);
    box_title(14 * 7 + 14, 36, "Which text file? (Return loads, Esc cancels)", 15, 64);
    for (;;) {
        for (i = 0; i < n; ++i) {
            lb_clear(11);
            ui_text(14 * 7 + 20, 8, 0, df[i].name);
            if (i == sel) lb_fill(14 * 7 + 14, 65 * 7 - 4, 0, 10, LB_XOR);
            present(50 + i * 11, 11, 15, 64);
        }
        k = wait_event();
        if (k == 1) {
            if (key == K_ESC) break;
            if (key == K_UP && sel) --sel;
            if (key == K_DOWN && sel + 1 < n) ++sel;
            if (key == K_RET) { done = 1; break; }
        } else {
            while (mbtn) mouse_poll();
            if (my >= 50 && my < 50 + n * 11 && mx > 14 * 7 && mx < 65 * 7) {
                k = (my - 50) / 11;
                if (k == sel) { done = 1; break; }
                sel = k;
            }
        }
        ptr_hide();
    }
    ptr_hide();
    unbox();
    if (!done) return;
    /* read it through its track/sector lists into the document at the caret */
    pshape = 2;
    ptr_hide(); ptr_show();
    delete_selection();
    p = cur;
    t = df[sel].t; s = df[sel].s;
    while (t && read_sec(t, s)) {
        memcpy(tsl, sect, 256);
        for (j = 0x0C; j < 0x100; j += 2) {
            if (!tsl[j]) { done = 2; break; }
            if (!read_sec(tsl[j], tsl[j + 1])) { done = 2; break; }
            for (e = 0; e < 256 && sect[e]; ++e) {
                sect[e] &= 0x7F;
                if (sect[e] != CR && sect[e] != TAB && (sect[e] < 0x20 || sect[e] > 0x7E)) sect[e] = ' ';
            }
            if (e && !raw_insert(p, sect, e)) { show_msg("The document is full."); done = 2; break; }
            p += e;
            total += e;
            if (e < 256) { done = 2; break; }       /* the end of the text */
        }
        if (done == 2) break;
        t = tsl[1]; s = tsl[2];
        done = 0;
    }
    pshape = 0;
    disp_shift(cur, total);
    cur = anc = p;
    modified = 1;
    text_moved();
}

/* ---------------------------------------------------------------------- */
/* Communications: a terminal through a Super Serial Card                  */

static unsigned char cslot = 2, cbaud = 3, ccap;
static const char *const bauds[5] = { "300", "1200", "2400", "9600", "19200" };
static const unsigned char bcode[5] = { 0x16, 0x18, 0x1A, 0x1E, 0x1F };
static const char *const slots[7] = { "1", "2", "3", "4", "5", "6", "7" };

#define TROWS 16
#define TTOP  (MENUH + 6)
static unsigned char tcol, trow;
static char tline[80];

static void term_flush(void)
{
    lb_clear(11);
    tline[tcol] = 0;
    ui_text(LEFTX, 8, 2, tline);            /* Mono */
    lb_fill(gx, gx + 6, 9, 9, LB_SET);      /* cursor */
    present(TTOP + trow * 11, 11, 0, 79);
}

static void __fastcall__ term_put(unsigned char c)
{
    if (c == LF) return;
    if (c == 8 || c == 0x7F) { if (tcol) --tcol; term_flush(); return; }
    if (c != CR && c >= 0x20 && c < 0x7F && tcol < 72) { tline[tcol++] = c; if (tcol < 72) return; }
    if (c == CR || tcol >= 72) {
        tline[tcol] = 0;
        term_flush();
        lb_clear(11);                       /* the line without the cursor */
        ui_text(LEFTX, 8, 2, tline);
        present(TTOP + trow * 11, 11, 0, 79);
        tcol = 0;
        if (trow + 1 < TROWS) ++trow;
        else {
            scr_copy(TTOP + 11, TTOP, (TROWS - 1) * 11, 0, 79);
            clear_rows(TTOP + (TROWS - 1) * 11, TTOP + TROWS * 11, 0, 79);
        }
    }
}

static void comm(void)
{
    unsigned char b, r, k, port, c, st;
    unsigned p;
    const unsigned char *rom;
    caret_hide();
    ptr_hide();
    draw_box(QC0, QC1, QY0, QY0 + 112);
    box_title(QX0, QY0 + 6, "Communications (Super Serial Card, 8 bits, no parity)", QC0 + 1, QC1 - 1);
    for (;;) {
        choice_row(QX0, QY0 + 24, "Card slot:", slots, 7, cslot - 1, QC0 + 1, QC1 - 1);
        choice_row(QX0, QY0 + 42, "Speed (baud):", bauds, 5, cbaud, QC0 + 1, QC1 - 1);
        choice_row(QX0, QY0 + 60, "Keep the text:", yn, 2, !ccap, QC0 + 1, QC1 - 1);
        two_buttons(QX0 + 330, QY0 + 86, "Connect", "Cancel");
        b = run_buttons(1);
        if (b < 2) break;
        if (b != 0xFD) continue;
        for (r = 0; r < 3; ++r)
            if (my >= QY0 + 24 + r * 18 && my < QY0 + 37 + r * 18) {
                if (r == 0) choice_row(QX0, QY0 + 24, "Card slot:", slots, 7, cslot - 1, QC0 + 1, QC1 - 1);
                if (r == 1) choice_row(QX0, QY0 + 42, "Speed (baud):", bauds, 5, cbaud, QC0 + 1, QC1 - 1);
                if (r == 2) choice_row(QX0, QY0 + 60, "Keep the text:", yn, 2, !ccap, QC0 + 1, QC1 - 1);
                k = choice_hit(r == 0 ? 7 : r == 1 ? 5 : 2);
                if (k == 0xFF) break;
                if (r == 0) cslot = k + 1;
                if (r == 1) cbaud = k;
                if (r == 2) ccap = !k;
            }
        ptr_hide();
    }
    ptr_hide();
    unbox();
    if (b == 1) return;
    rom = (const unsigned char *)(0xC000 + cslot * 0x100);
    if (rom[5] != 0x38 || rom[7] != 0x18 || rom[0x0B] != 0x01 || rom[0x0C] != 0x31) {
        strcpy(msg, "There is no Super Serial Card in slot ");
        c = strlen(msg);
        msg[c] = '0' + cslot; msg[c + 1] = 0;
        strcat(msg, ".");
        show_msg(msg);
        return;
    }
    port = cslot << 4;
    POKE(0xC08B + port, 0x10 | bcode[cbaud]);  /* 8 data bits, 1 stop bit */
    POKE(0xC08A + port, 0x0B);                   /* no parity, no interrupts, DTR on */
    /* terminal screen */
    clear_rows(MENUH, BOTY, 0, 79);
    lb_clear(11);
    ui_text(LEFTX, 8, F_BOLD, "Terminal - type to send.  \x7F-period ends.");
    if (ccap) ui_text(gx, 8, F_ITAL, "  (the text received is kept)");
    present(MENUH + 1, 11, 0, 79);
    tcol = trow = 0;
    p = cur;
    term_flush();
    for (;;) {
        st = PEEK(0xC089 + port);
        if (st & 0x08) {                        /* a byte came in */
            c = PEEK(0xC088 + port) & 0x7F;
            term_put(c);
            if (c == 8 || c == 0x7F) term_flush();
            if (ccap && (c == CR || c == TAB || (c >= 0x20 && c < 0x7F)) && tlen < CAP - 4) {
                raw_insert(p, &c, 1);
                ++p;
            }
        }
        if (PEEK(KBD) & 0x80) {
            c = PEEK(KBD) & 0x7F;
            k = PEEK(OAPPLE) & 0x80;
            POKE(KBDSTRB, 0);
            if (k && c == '.') break;
            while (!(PEEK(0xC089 + port) & 0x10)) ;   /* wait to send */
            POKE(0xC088 + port, c);
        }
    }
    POKE(0xC08A + port, 0x0A);                   /* DTR off */
    if (p != cur) {
        disp_shift(cur, p - cur);
        cur = anc = p;
        modified = 1;
    }
    invalidate_layout();
    redraw_all();
}

/* ---------------------------------------------------------------------- */
/* BASIC.SYSTEM: the quit code is replaced so that BYE comes back here;   */
/* RetroWrite puts the original back when it starts.                      */

void basic_launch(void)
{
    static char bp[66];
    int fd;
    unsigned char n;
    unsigned off;
    char *s;
    /* find BASIC.SYSTEM: our folder, then the volume */
    prog_file("/BASIC.SYSTEM");
    strcpy(bp + 1, tmppath);
    fd = open(bp + 1, O_RDONLY);
    if (fd < 0) {
        strcpy(bp + 1, progdir);
        s = strchr(bp + 2, '/');
        if (s) *s = 0;
        strcat(bp + 1, "/BASIC.SYSTEM");
        fd = open(bp + 1, O_RDONLY);
    }
    if (fd < 0) return;                 /* checked before quitting */
    close(fd);
    bp[0] = strlen(bp + 1);
    /* save the original quit code, once */
    if (memcmp((void *)0xD100, qstub, 8)) {
        prog_file("/RW2.QUIT");
        _filetype = PRODOS_T_BIN;
        _auxtype = 0;
        fd = open(tmppath, O_WRONLY | O_CREAT | O_TRUNC);
        if (fd < 0) return;
        memcpy(iob, (void *)0xD100, 256);
        write(fd, iob, 256);
        memcpy(iob, (void *)0xD200, 512);
        write(fd, iob, 512);
        close(fd);
    }
    /* our quit code, with the path of RW2.SYSTEM */
    off = (unsigned)qstub_len;
    n = (unsigned char)off;
    memcpy(iob, qstub, n);
    prog_file("/RW2.SYSTEM");
    off = (unsigned)qpath_off;
    s = (char *)iob + off;
    s[0] = strlen(tmppath);
    strcpy(s + 1, tmppath);
    (void)PEEK(0xC083);
    (void)PEEK(0xC083);
    memcpy((void *)0xD100, iob, n);
    (void)PEEK(0xC080);
    /* BASIC starts with the disk's top level as its prefix */
    strcpy(tmppath, progdir);
    s = strchr(tmppath + 1, '/');
    if (s) *s = 0;
    chdir(tmppath);
    basic_go(bp);
}

/* does BASIC.SYSTEM exist? (asked before quitting) */
static unsigned char basic_ok(void)
{
    int fd;
    char *s;
    prog_file("/BASIC.SYSTEM");
    fd = open(tmppath, O_RDONLY);
    if (fd < 0) {
        strcpy(tmppath, progdir);
        s = strchr(tmppath + 1, '/');
        if (s) *s = 0;
        strcat(tmppath, "/BASIC.SYSTEM");
        fd = open(tmppath, O_RDONLY);
    }
    if (fd < 0) {
        show_msg("BASIC.SYSTEM was not found. Copy it into\nthe RetroWrite II folder or the disk's top level.");
        return 0;
    }
    close(fd);
    return alert("Leave RetroWrite II for BASIC.SYSTEM?\nType BYE in BASIC to come back.", "Go", "Cancel", 0) == 0;
}

unsigned char __fastcall__ acc_command(unsigned char c)
{
    switch (c) {
    case CM_PREFS: prefs(); break;
    case CM_DOSLOAD: dos_load(); break;
    case CM_COMM: comm(); break;
    case CM_BASIC: return basic_ok();
    }
    return 1;
}
