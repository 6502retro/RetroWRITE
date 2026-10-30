/*
** RetroWrite II - overlay 5: Page Setup, Print, Print Several
**
** Two ways to print:
**  - as on screen: every line is drawn exactly as on the screen and sent
**    as dot graphics (80 dots per inch across, each screen row = 2 dots
**    down at 72 dots per inch), so fonts, sizes and styles print as shown;
**  - faster: the printer's own font, with bold, underline, italic and
**    wide text switched by the printer's codes. Lines are wrapped again for
**    10 characters per inch.
** Printers: ImageWriter II, Epson (ESC/P) or plain text (any printer).
*/
#pragma code-name (push, "OVL5")
#pragma rodata-name (push, "OVL5")
#pragma data-name (push, "OVL5")
#pragma bss-name (push, "OVL5BSS")

#include <string.h>
#include <stdlib.h>
#include "rw.h"

extern unsigned char pb_row, pb_dots[4];
extern unsigned pb_x0, pb_x1;
unsigned char __fastcall__ prt_open(unsigned char slot);
void __fastcall__ prt_byte(unsigned char c);
void prt_band(void);

#define IW    0
#define EPSON 1
#define PLAIN 2

#define ESC 27

/* page geometry in dots (1/72 inch); the body sits between header and footer */
#define PAGE_TOP   48           /* header area */
#define PAGE_BODY  672          /* end of the body */
#define PAGE_END   720          /* 10 inches; the printer's form feed does the rest */

static unsigned ypos;           /* dots used on this page */
static unsigned char page, stop;
static char num[8], msg[80];

static void __fastcall__ pstr(const char *s)
{
    while (*s) prt_byte(*s++);
}

static void __fastcall__ esc(unsigned char c)
{
    prt_byte(ESC);
    prt_byte(c);
}

static void crlf(void)
{
    prt_byte(13);
    prt_byte(10);
}

/* Esc pressed? */
static unsigned char check_stop(void)
{
    if ((PEEK(KBD) & 0x7F) == K_ESC && (PEEK(KBD) & 0x80)) {
        POKE(KBDSTRB, 0);
        stop = 1;
    }
    return stop;
}

/* header or footer text: '#' becomes the page number */
static const char *__fastcall__ hf_text(const char *t)
{
    static char out[48];
    unsigned char i = 0;
    utoa(page, num, 10);
    for (; *t && i < 40; ++t) {
        if (*t == '#') { strcpy(out + i, num); i += strlen(num); }
        else out[i++] = *t;
    }
    out[i] = 0;
    return out;
}

/* ---------------------------------------------------------------------- */
/* Progress box                                                            */

static void show_progress(void)
{
    lb_clear(11);
    ui_text(17 * 7 + 12, 8, 0, "Printing page ");
    utoa(page, num, 10);
    ui_text(gx, 8, F_BOLD, num);
    ui_text(gx, 8, 0, ".   Press Esc to stop.");
    present(88, 11, 18, 61);
}

/* ---------------------------------------------------------------------- */
/* Graphics (as on screen)                                                 */

/* send LB rows 0..h-1 (h a multiple of 4) as bands; x range of the text */
static void __fastcall__ gemit(unsigned char h)
{
    unsigned char r, i;
    unsigned n = 80 + (LEFTX + rmargin + 8 - LEFTX);    /* 1 inch of paper margin */
    for (r = 0; r < h; r += 4) {
        if (prt_type == IW) {
            esc('G');
            num[0] = '0' + n / 1000;
            num[1] = '0' + n / 100 % 10;
            num[2] = '0' + n / 10 % 10;
            num[3] = '0' + n % 10;
            num[4] = 0;
            pstr(num);
        } else {
            esc('*');
            prt_byte(4);                /* 80 dots per inch */
            prt_byte(n & 0xFF);
            prt_byte(n >> 8);
        }
        for (i = 0; i < 80; ++i) prt_byte(0);
        pb_row = r;
        pb_x0 = LEFTX;
        pb_x1 = LEFTX + rmargin + 8;
        prt_band();
        crlf();
        ypos += 8;
    }
}

/* blank paper down to dot position y (in steps of 8) */
static void __fastcall__ gskip(unsigned y)
{
    while (ypos < y) { prt_byte(10); ypos += 8; }
}

/* header or footer line, centered */
static void __fastcall__ ghf(const char *t)
{
    if (!t[0]) return;
    t = hf_text(t);
    lb_clear(12);
    ui_text(LEFTX + (lmargin + rmargin - text_w(0, t)) / 2, 8, 0, t);
    gemit(12);
}

static void gpage_start(void)
{
    ypos = 0;
    ghf(doc_head);
    gskip(PAGE_TOP);
    show_progress();
}

static void gpage_end(void)
{
    gskip(PAGE_BODY + 8);
    if (doc_foot[0]) ghf(doc_foot);
    else if (prt_pnum) { ghf("- # -"); }
    prt_byte(12);                       /* form feed */
    ++page;
}

static void gdoc(void)
{
    unsigned p = 0, w, slack;
    unsigned char fmt = 0, pa = 0, n, i, h, hh, a, d, ns, cr, nf, np, q, pb;
    if (prt_type == IW) {
        esc('N');                       /* 80 dots per inch */
        esc('T'); pstr("16");           /* line feed = 8 dots */
        esc('>');                       /* one direction: straight columns */
    } else {
        esc('@');
        esc('3'); prt_byte(24);         /* line feed = 24/216 inch = 8 dots */
        esc('U'); prt_byte(1);
    }
    gpage_start();
    while (p < tlen && !check_stop()) {
        n = tlen - p > 255 ? 255 : (unsigned char)(tlen - p);
        fetch(p, TB, n);
        lm_n = n;
        gx = LEFTX + lmargin;
        gfmt = fmt;
        lm_pa = pa;
        line_engine(0);
        i = lm_i; nf = lm_nf; np = lm_np; a = lm_asc; d = lm_desc;
        w = lm_w; ns = lm_ns; cr = lm_cr; q = lm_pa;
        h = a + d + 1;
        h += (q & 0x30) == 0x10 ? h >> 1 : (q & 0x30) == 0x20 ? h : 0;
        pb = (i && TB[i - 1] == 0x0C);  /* page break at the end of the line */
        hh = (h + 3) & 0xFC;
        if (ypos + hh * 2 > PAGE_BODY) {
            gpage_end();
            gpage_start();
        }
        lb_clear(hh);
        gx = LEFTX + lmargin;
        slack = (w < LEFTX + rmargin) ? LEFTX + rmargin - w : 0;
        lm_jn = 0;
        switch (q & 0x0C) {
        case 0x04: gx += slack >> 1; break;
        case 0x08: gx += slack; break;
        case 0x0C:
            if (ns && slack < 256) { lm_jn = ns; lm_jx = (unsigned char)slack / ns; lm_jr = (unsigned char)slack % ns; }
        }
        gfmt = fmt;
        lm_pa = pa;
        gbase = a - 1;
        lm_n = i;
        lm_s0 = lm_s1 = 0xFF;
        lm_x0 = lm_x1 = 0xFFFF;
        line_engine(3);
        gemit(hh);
        p += i;
        fmt = nf;
        pa = np;
        if (pb) { gpage_end(); gpage_start(); }
        if (cr == 2 && n < 255) break;
    }
    gpage_end();
    if (prt_type == IW) { esc('A'); esc('<'); }
    else { esc('2'); esc('U'); prt_byte(0); }
}

/* ---------------------------------------------------------------------- */
/* Faster: the printer's own font                                          */

static unsigned char ob[200], obn, col, lines, curst;
static unsigned char wcols, lcol;

/* switch the printer to character format f */
static unsigned char __fastcall__ small(unsigned char f)    /* 1 super, 2 sub */
{
    return (f & F_SIZE) == F_SIZE ? (f & F_BOLD ? 2 : 1) : 0;
}

static void __fastcall__ pstyle(unsigned char f)
{
    unsigned char sm, osm, n, o;
    if (prt_type == PLAIN) return;
    sm = small(f);
    osm = small(curst);
    n = !sm && (f & F_BOLD);
    o = !osm && (curst & F_BOLD);
    if (n != o) {
        if (prt_type == IW) esc(n ? '!' : '"');
        else esc(n ? 'E' : 'F');
    }
    if ((f ^ curst) & F_UND) {
        if (prt_type == IW) esc(f & F_UND ? 'X' : 'Y');
        else { esc('-'); prt_byte(f & F_UND ? 1 : 0); }
    }
    if (((f ^ curst) & F_ITAL) && prt_type == EPSON) esc(f & F_ITAL ? '4' : '5');
    n = !sm && (f & F_SIZE);            /* 18 and 27 point: wide letters */
    o = !osm && (curst & F_SIZE);
    if (n != o) {
        if (prt_type == IW) prt_byte(n ? 14 : 15);
        else { esc('W'); prt_byte(n ? 1 : 0); }
    }
    if (sm != osm) {
        if (osm) esc(prt_type == IW ? 'z' : 'T');
        if (sm) {
            if (prt_type == IW) esc(sm == 2 ? 'y' : 'x');
            else { esc('S'); prt_byte(sm == 2); }
        }
    }
    curst = f;
}

static void __fastcall__ spaces(unsigned char n)
{
    while (n--) prt_byte(' ');
}

static void __fastcall__ thf(const char *t)
{
    unsigned char n;
    if (t[0]) {
        t = hf_text(t);
        n = strlen(t);
        spaces(lcol + (n < wcols ? (wcols - n) / 2 : 0));
        pstr(t);
    }
    crlf();
}

static void tpage_start(void)
{
    thf(doc_head);
    crlf();
    lines = 2;
    show_progress();
}

static void tpage_end(void)
{
    while (lines < 58) { crlf(); ++lines; }
    thf(doc_foot[0] ? doc_foot : prt_pnum ? "- # -" : "");
    prt_byte(12);
    ++page;
}

/* print ob[0..len) as one line with alignment a */
static void __fastcall__ tline(unsigned char len, unsigned char a, unsigned char vis)
{
    unsigned char i, c, st = curst;
    if (lines >= 56) { tpage_end(); tpage_start(); }
    pstyle(0);
    i = lcol;
    if (a == 0x04 && vis < wcols) i += (wcols - vis) / 2;
    if (a == 0x08 && vis < wcols) i += wcols - vis;
    spaces(i);
    pstyle(st);
    for (i = 0; i < len; ++i) {
        c = ob[i];
        if (c & 0x80) pstyle(c & 0x7F);
        else prt_byte(c);
    }
    crlf();
    ++lines;
}

/* visible columns in ob[from..to) */
static unsigned char __fastcall__ vis_cols(unsigned char from, unsigned char to)
{
    unsigned char v = 0;
    for (; from < to; ++from) if (!(ob[from] & 0x80)) ++v;
    return v;
}

static void tdoc(void)
{
    unsigned p, k;
    unsigned char c, i, n, al = 0, lastsp = 0, j;
    wcols = (rmargin - lmargin) / 8;
    lcol = lmargin / 8;
    curst = 0;
    if (prt_type == IW) { esc('N'); esc('A'); }
    else if (prt_type == EPSON) esc('@');
    tpage_start();
    obn = col = 0;
    for (p = 0; p < tlen && !check_stop(); p += n) {
        n = tlen - p > 128 ? 128 : (unsigned char)(tlen - p);
        fetch(p, TB + 128, n);
        for (i = 0; i < n; ++i) {
            c = TB[128 + i];
            if (c & 0x80) {
                if ((c & 3) == 3) al = c & 0x0C;
                else if (obn < 199) ob[obn++] = c;
                continue;
            }
            if (c == CR || c == 0x0C) {
                tline(obn, al, col);
                obn = col = lastsp = 0;
                al = 0;
                if (c == 0x0C) { tpage_end(); tpage_start(); }
                continue;
            }
            if (c == TAB) {             /* spaces up to the next stop */
                k = 0;
                for (j = 0; j < 8 && dtabs[j]; ++j)
                    if (dtabs[j] > lmargin + col * 8) { k = (dtabs[j] - lmargin) / 8; break; }
                if (!k) k = (col / 5 + 1) * 5;
                c = ' ';
                while (col + 1 < k && col < wcols && obn < 199) { ob[obn++] = ' '; ++col; }
            }
            if (obn >= 199) continue;
            ob[obn++] = c;
            ++col;
            if (c == ' ') lastsp = obn;
            if (col > wcols) {          /* wrap at the last space */
                if (lastsp) {
                    tline(lastsp - 1, al == 0x0C ? 0 : al, vis_cols(0, lastsp - 1));
                    memmove(ob, ob + lastsp, obn - lastsp);
                    obn -= lastsp;
                } else {
                    tline(obn - 1, al == 0x0C ? 0 : al, wcols);
                    ob[0] = c;
                    obn = 1;
                }
                col = vis_cols(0, obn);
                lastsp = 0;
            }
        }
    }
    if (obn) tline(obn, al, col);
    pstyle(0);
    tpage_end();
}

/* ---------------------------------------------------------------------- */

/* print the document in memory; 0 if stopped or no printer */
static unsigned char print_doc(void)
{
    stop = 0;
    page = 1;
    if (prt_open(prt_slot)) {
        num[0] = '0' + prt_slot;
        num[1] = 0;
        strcpy(msg, "No printer card was found in slot ");
        strcat(msg, num);
        strcat(msg, ".\nChoose the slot in Page Setup.");
        show_msg(msg);
        return 0;
    }
    if (prt_type == IW) { pb_dots[0] = 0x03; pb_dots[1] = 0x0C; pb_dots[2] = 0x30; pb_dots[3] = 0xC0; }
    else { pb_dots[0] = 0xC0; pb_dots[1] = 0x30; pb_dots[2] = 0x0C; pb_dots[3] = 0x03; }
    caret_hide();
    ptr_hide();
    scr_save(16, 63, 80, 106);
    draw_box(16, 63, 80, 106);
    pshape = 2;
    ruler_apply();
    if (prt_quality == 0 && prt_type != PLAIN) gdoc();
    else tdoc();
    pshape = 0;
    ptr_hide();
    scr_restore();
    return !stop;
}

/* ---------------------------------------------------------------------- */
/* Page Setup                                                              */

#define PC0 8
#define PC1 71
#define PX0 (PC0 * 7)
#define PY0 30

static const char *const opt_lab[4] = { "Printer:", "Slot:", "Print:", "Page numbers:" };
static const char *const opt_txt[4][7] = {
    { "ImageWriter II", "Epson", "Plain text", 0 },
    { "1", "2", "3", "4", "5", "6", "7" },
    { "As on screen", "Faster (printer's font)", 0 },
    { "Yes", "No", 0 },
};
static unsigned ox0[4][7], ox1[4][7];

static unsigned char __fastcall__ opt_val(unsigned char r)
{
    switch (r) {
    case 0: return prt_type;
    case 1: return prt_slot - 1;
    case 2: return prt_quality;
    }
    return !prt_pnum;
}

static void __fastcall__ draw_opt(unsigned char r)
{
    unsigned char k, sel = opt_val(r);
    unsigned x;
    lb_clear(13);
    ui_text(PX0 + 14, 9, F_BOLD, opt_lab[r]);
    x = PX0 + 134;
    for (k = 0; k < 7 && opt_txt[r][k]; ++k) {
        ox0[r][k] = x;
        ui_text(x + 4, 9, 0, opt_txt[r][k]);
        ox1[r][k] = gx + 4;
        if (k == sel) lb_fill(x, gx + 4, 0, 12, LB_XOR);
        else frame_rect(x, gx + 4, 0, 12);
        x = gx + 12;
    }
    present(PY0 + 22 + r * 18, 13, PC0 + 1, PC1 - 1);
}

static void __fastcall__ draw_hf(unsigned char r)
{
    lb_clear(13);
    ui_text(PX0 + 14, 9, F_BOLD, r ? "Footer:" : "Header:");
    frame_rect(PX0 + 134, PX0 + 430, 0, 12);
    ui_text(PX0 + 140, 9, 0, r ? doc_foot : doc_head);
    present(PY0 + 94 + r * 18, 13, PC0 + 1, PC1 - 1);
}

static void page_setup(void)
{
    unsigned char r, k, b, ot = prt_type, os = prt_slot, oq = prt_quality, op = prt_pnum;
    static char oh[40], of[40];
    strcpy(oh, doc_head);
    strcpy(of, doc_foot);
    caret_hide();
    ptr_hide();
    draw_box(PC0, PC1, PY0, PY0 + 160);     /* too big to save: redrawn after */
    lb_clear(11);
    ui_text(PX0 + 14, 8, F_BOLD, "Page Setup");
    ui_text(PX0 + 140, 8, F_ITAL, "(# in a header or footer = page number)");
    present(PY0 + 6, 11, PC0 + 1, PC1 - 1);
    for (r = 0; r < 4; ++r) draw_opt(r);
    draw_hf(0);
    draw_hf(1);
    nbtn = 2;
    blabel[0] = "OK"; blabel[1] = "Cancel";
    for (b = 0; b < 2; ++b) {
        bx0[b] = PX0 + 336 - b * 92;
        bx1[b] = bx0[b] + 84;
        by[b] = PY0 + 136;
        draw_button(bx0[b], bx1[b], by[b], blabel[b], b == 0);
    }
    for (;;) {
        b = run_buttons(1);
        if (b == 0) break;
        if (b == 1) {                   /* Cancel */
            prt_type = ot; prt_slot = os; prt_quality = oq; prt_pnum = op;
            strcpy(doc_head, oh);
            strcpy(doc_foot, of);
            break;
        }
        if (b != 0xFD) continue;        /* a click elsewhere */
        for (r = 0; r < 4; ++r) {
            if (my < PY0 + 22 + r * 18 || my >= PY0 + 35 + r * 18) continue;
            for (k = 0; k < 7 && opt_txt[r][k]; ++k)
                if (mx >= ox0[r][k] && mx < ox1[r][k]) {
                    switch (r) {
                    case 0: prt_type = k; break;
                    case 1: prt_slot = k + 1; break;
                    case 2: prt_quality = k; break;
                    case 3: prt_pnum = !k; break;
                    }
                    ptr_hide();
                    draw_opt(r);
                }
        }
        for (r = 0; r < 2; ++r)
            if (my >= PY0 + 94 + r * 18 && my < PY0 + 107 + r * 18 && mx >= PX0 + 134 && mx < PX0 + 430) {
                strcpy(msg, r ? doc_foot : doc_head);
                ptr_hide();
                if (ask_text(r ? "Footer (# = page number):" : "Header (# = page number):", msg, 39))
                    strcpy(r ? doc_foot : doc_head, msg);
                draw_hf(r);
            }
    }
    if (strcmp(oh, doc_head) || strcmp(of, doc_foot)) modified = 1;
    ptr_hide();
    redraw_all();
    refresh();
}

/* ---------------------------------------------------------------------- */

unsigned char __fastcall__ print_command(unsigned char c)
{
    static const char *const ptn[3] = { "ImageWriter II", "Epson printer", "printer" };
    switch (c) {
    case CM_PSETUP: page_setup(); return 1;
    case CM_PRINTNOW: return print_doc();
    case CM_PRINT:
        strcpy(msg, "Print this document on the\n");
        strcat(msg, ptn[prt_type]);
        strcat(msg, " in slot ");
        num[0] = '0' + prt_slot; num[1] = 0;
        strcat(msg, num);
        strcat(msg, "?");
        switch (alert(msg, "Print", "Setup...", "Cancel")) {
        case 0: return print_doc();
        case 1: page_setup(); return 1;
        }
    }
    return 0;
}
