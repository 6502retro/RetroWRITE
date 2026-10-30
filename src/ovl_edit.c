/*
** RetroWrite II - overlay 4: find/replace, clipboard, styles, glossary
*/
#pragma code-name (push, "OVL4")
#pragma rodata-name (push, "OVL4")
#pragma data-name (push, "OVL4")
#pragma bss-name (push, "OVL4BSS")

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <apple2enh.h>
#include "rw.h"

static unsigned char __fastcall__ lowc(unsigned char c)
{
    return (c >= 'A' && c <= 'Z') ? c + 32 : c;
}

/* find findstr starting at or after 'from' and before 'to'. Markers inside
   the text are skipped. Sets found_end. */
static unsigned found_end;
static unsigned __fastcall__ search(unsigned from, unsigned to)
{
    unsigned p, q;
    unsigned char j, L, c0, c;
    L = (unsigned char)strlen(findstr);
    if (!L) return NOLINE;
    c0 = lowc(findstr[0]);
    for (p = from; p < to; ++p) {
        if ((p & 63) == 0 && (PEEK(KBD) & 0x80) && (PEEK(KBD) & 0x7F) == K_ESC) return NOLINE;
        c = tch(p);
        if (lowc(c) != c0) continue;
        q = p + 1;
        for (j = 1; j < L && q < tlen; ++q) {
            c = tch(q);
            if (c & 0x80) continue;
            if (lowc(c) != lowc(findstr[j])) break;
            ++j;
        }
        if (j == L) { found_end = q; return p; }
    }
    return NOLINE;
}

static unsigned char __fastcall__ find_from(unsigned start, unsigned char wrapmsg)
{
    unsigned p;
    unsigned char wrapped = 0;
    p = search(start, tlen);
    if (p == NOLINE) { p = search(1, start); wrapped = 1; }
    if (p == NOLINE) {
        if (wrapmsg) alert("Not found.", "OK", 0, 0);
        return 0;
    }
    {
        unsigned olo = cur < anc ? cur : anc, ohi = cur < anc ? anc : cur;
        anc = p;
        cur = found_end;
        sel_changed(olo, ohi);
    }
    (void)wrapped;
    return 1;
}

void find_next(void)
{
    if (!findstr[0]) { cmd_find(0); return; }
    find_from(cur > anc ? cur : anc, 1);
    scroll_to_caret();
}

/* replace the selection with replstr, keeping the format that follows */
static void replace_sel(void)
{
    unsigned lo = anc < cur ? anc : cur, hi = anc < cur ? cur : anc;
    unsigned char fend = fmt_at(hi), m;
    unsigned n = strlen(replstr);
    raw_delete(lo, hi - lo);
    raw_insert(lo, (const unsigned char *)replstr, n);
    disp_shift(lo, -(int)(hi - lo));
    cur = anc = lo + n;
    if (fmt_at(cur) != fend) {
        m = 0x80 | fend;
        raw_insert(cur, &m, 1);
        ++n;
    }
    disp_shift(lo, n);
    text_moved();
}

void cmd_find(unsigned char replace)
{
    unsigned char r;
    unsigned n = 0, p;
    static char msg[40];
    if (!ask_text("Find what?", findstr, 30)) return;
    if (!replace) { find_next(); return; }
    if (!ask_text("Replace it with?", replstr, 30)) return;
    if (!find_from(cur < anc ? cur : anc, 1)) return;
    for (;;) {
        scroll_to_caret();
        refresh();
        r = alert("Replace this one?", "Replace", "All", "Stop");
        if (r == 2) break;
        if (r == 0) {
            replace_sel();
            ++n;
            if (!find_from(cur, 0)) break;
            continue;
        }
        /* replace all, from the top, never revisiting replaced text */
        p = 1;
        while ((p = search(p, tlen)) != NOLINE) {
            anc = p;
            cur = found_end;
            replace_sel();
            ++n;
            p = cur;
        }
        break;
    }
    invalidate_layout();
    scroll_to_caret();
    refresh();
    strcpy(msg, "Replaced ");
    utoa(n, msg + 9, 10);
    strcat(msg, n == 1 ? " time." : " times.");
    alert(msg, "OK", 0, 0);
}

/* ---------------------------------------------------------------------- */
/* Clipboard: a file in the RetroWrite II folder on the hard disk          */

static void clip_path(void)
{
    strcpy(tmppath, progdir);
    strcat(tmppath, "/RW2.CLIPBOARD");
}

static void copy_selection(void)
{
    unsigned lo = sel_lo(), hi = sel_hi(), p, n;
    int fd;
    if (lo == hi) return;
    clip_path();
    remove(tmppath);
    _filetype = PRODOS_T_BIN;
    _auxtype = 0;
    fd = open(tmppath, O_WRONLY | O_CREAT | O_TRUNC);
    if (fd < 0) { show_msg("Could not write the clipboard file."); return; }
    iob[0] = 0x80 | fmt_at(lo);
    write(fd, iob, 1);
    for (p = lo; p < hi; p += n) {
        n = hi - p > 512 ? 512 : hi - p;
        fetch(p, iob, n);
        write(fd, iob, n);
    }
    close(fd);
}

static void paste_clip(void)
{
    int fd, n;
    unsigned p;
    unsigned char m;
    clip_path();
    fd = open(tmppath, O_RDONLY);
    if (fd < 0) { beep(); return; }
    delete_selection();
    p = cur;
    m = 0x80 | fmt_at(p);              /* format to restore after the paste */
    while ((n = read(fd, iob, 512)) > 0) {
        if (!raw_insert(p, iob, n)) { alert("The document is full.", "OK", 0, 0); break; }
        p += n;
    }
    close(fd);
    raw_insert(p, &m, 1);
    disp_shift(cur, p + 1 - cur);
    cur = anc = p + 1;
    text_moved();
}

/* ---------------------------------------------------------------------- */
/* Fonts, sizes and styles                                                 */

static unsigned char sk, sv, son;

static unsigned char __fastcall__ apply_fmt(unsigned char f)
{
    unsigned char small = (f & F_SIZE) == F_SIZE;     /* superscript or subscript */
    switch (sk) {
    case 0: return (f & ~F_FONT) | sv;
    case 1: if (small) f &= ~F_BOLD;
            return (f & ~F_SIZE) | (sv << 2);
    case 2: if (small && sv == F_BOLD) return f;        /* bold marks subscript */
            return son ? (f | sv) : (f & ~sv);
    case 4: f &= ~(F_SIZE | F_BOLD);                    /* sv: 0 super, F_BOLD sub */
            return son ? f | F_SIZE | sv : f;
    }
    return f & (small ? F_FONT : F_FONT | F_SIZE);
}

/* kind 0 font, 1 size, 2 toggle style bits, 3 plain, 4 superscript/subscript */
void __fastcall__ set_style(unsigned char kind, unsigned char val)
{
    unsigned lo = sel_lo(), hi = sel_hi(), p;
    unsigned char fend, k, i, m;
    sk = kind;
    sv = val;
    m = (lo == hi) ? tf : fmt_at(skip_fwd(lo) + 1);
    son = kind == 4 ? (m & (F_SIZE | F_BOLD)) != (F_SIZE | val) : !(m & val);
    if (lo == hi) {                     /* applies to what is typed next */
        tf = apply_fmt(tf);
        return;
    }
    fend = fmt_at(hi);
    for (p = lo; p < hi; p += k) {      /* rewrite markers inside */
        k = (hi - p) > 128 ? 128 : (unsigned char)(hi - p);
        fetch(p, TB, k);
        for (i = 0; i < k; ++i)
            if ((TB[i] & 0x80) && (TB[i] & 3) != 3)      /* not paragraph markers */
                aux_poke(phys(p + i), 0x80 | apply_fmt(TB[i] & 0x7F));
    }
    m = 0x80 | fend;
    raw_insert(hi, &m, 1);
    tf = apply_fmt(fmt_at(lo));
    m = 0x80 | tf;
    raw_insert(lo, &m, 1);
    if (cur >= anc) { anc = lo; cur = hi + 2; } else { cur = lo; anc = hi + 2; }
    disp_shift(lo, -(int)(hi - lo));
    disp_shift(lo, hi - lo + 2);
    text_moved();
}

/* ---------------------------------------------------------------------- */
/* Glossary: GLOSSARY (a text file in the RetroWrite II folder) has one   */
/* entry per line, "k=text": Control-G then k inserts the text.           */

static unsigned char gtxt[200];

/* find entry k (0: list the keys into gtxt); returns its length */
static unsigned char __fastcall__ gloss_find(unsigned char k)
{
    int fd, n;
    unsigned char i, c, st = 0, len = 0, key = 0;
    strcpy(tmppath, progdir);
    strcat(tmppath, "/GLOSSARY");
    fd = open(tmppath, O_RDONLY);
    if (fd < 0) return 0xFF;
    while ((n = read(fd, iob, 512)) > 0) {
        for (i = 0; i < (unsigned char)n || (n == 512 && i == 0 && 0); ++i) {
            c = iob[i] & 0x7F;
            if (c == LF) continue;
            if (c == CR) { if (st == 2 && key == k) goto done; st = 0; continue; }
            if (st == 0) { key = c; st = 1; continue; }
            if (st == 1) {
                st = (c == '=') ? 2 : 3;
                if (st == 2 && !k && len < 190) { gtxt[len++] = key; gtxt[len++] = ' '; }
                continue;
            }
            if (st == 2 && key == k && k && len < 199 && c >= 0x20) gtxt[len++] = c;
        }
        if (n < 512) break;
    }
    if (st != 2 || key != k) len = k ? 0 : len;
done:
    close(fd);
    gtxt[len] = 0;
    return len;
}

static void glossary(void)
{
    unsigned char k, n, m;
    unsigned p;
    caret_hide();
    ptr_hide();
    scr_save(16, 63, 70, 96);
    draw_box(16, 63, 70, 96);
    lb_clear(11);
    ui_text(16 * 7 + 14, 8, 0, "Glossary: press the entry's key  (? lists them)");
    present(78, 11, 17, 62);
    while (wait_event() != 1) ;
    k = key;
    ptr_hide();
    scr_restore();
    if (k == K_ESC) return;
    if (k >= 'a' && k <= 'z') k -= 32;
    if (k == '?') {
        n = gloss_find(0);
        if (n == 0xFF) goto none;
        strcpy((char *)TB, "Glossary keys:  ");
        strncat((char *)TB, (char *)gtxt, 180);
        alert((char *)TB, "OK", 0, 0);
        return;
    }
    n = gloss_find(k);
    if (n == 0xFF) goto none;
    if (!n) {                               /* try the key as typed */
        n = gloss_find(key);
        if (!n || n == 0xFF) { beep(); return; }
    }
    delete_selection();
    p = cur;
    m = 0;
    if (fmt_at(p) != tf) {                  /* in the typing style */
        m = 0x80 | tf;
        raw_insert(p, &m, 1);
        m = 1;
    }
    if (!raw_insert(p + m, gtxt, n)) { alert("The document is full.", "OK", 0, 0); return; }
    disp_shift(p, n + m);
    cur = anc = p + n + m;
    modified = 1;
    text_moved();
    return;
none:
    alert("There is no GLOSSARY file in the\nRetroWrite II folder. Use Edit Glossary\nto make one: lines like  A=Apple IIe", "OK", 0, 0);
}

void __fastcall__ edit_command(unsigned char c)
{
    switch (c) {
    case CM_CUT: copy_selection(); delete_selection(); break;
    case CM_COPY: copy_selection(); break;
    case CM_PASTE: paste_clip(); break;
    case CM_FIND: cmd_find(0); break;
    case CM_FINDNEXT: find_next(); break;
    case CM_REPLACE: cmd_find(1); break;
    case CM_SANS: case CM_SERIF: case CM_MONO: set_style(0, c - CM_SANS); break;
    case CM_S9: case CM_S18: case CM_S27: set_style(1, c - CM_S9); break;
    case CM_PLAIN: set_style(3, 0); break;
    case CM_BOLD: set_style(2, F_BOLD); break;
    case CM_ITALIC: set_style(2, F_ITAL); break;
    case CM_UNDER: set_style(2, F_UND); break;
    case CM_SUPER: set_style(4, 0); break;
    case CM_GLOSSARY: glossary(); break;
    case CM_SUB: set_style(4, F_BOLD); break;
    case CM_CLEAR: delete_selection(); break;
    case CM_PGBREAK: insert_char(CR); insert_char(0x0C); break;  /* the break is a line of its own */
    case CM_TOP: cur = anc = 1; top = 0; topf = topp = 0; layout_ok = 0; break;
    case CM_END: cur = anc = tlen; break;
    case CM_WLEFT:
        while (cur > 1 && !isword(tch(cur - 1))) --cur;
        while (cur > 1 && isword(tch(cur - 1))) --cur;
        anc = cur;
        break;
    case CM_WRIGHT:
        while (cur < tlen && isword(tch(cur))) ++cur;
        while (cur < tlen && !isword(tch(cur))) ++cur;
        anc = cur;
        break;
    case CM_DELFWD:
        if (!delete_selection()) {
            unsigned q = skip_fwd(cur);
            if (q < tlen) delete_range(q, 1);
        }
        break;
    }
}

