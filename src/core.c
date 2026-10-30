/*
** RetroWrite II - graphics edition: resident core
** document, layout, rendering, editing, mouse, menus, dialogs, overlays
*/
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "rw.h"

/* ---------------------------------------------------------------------- */
/* Globals                                                                 */

unsigned gs, ge, tlen;
unsigned cur, anc;
unsigned top;
unsigned char topf, topp, modified, tf, doctype, quit_now;
char docpath[65];
unsigned char key, oa;
/* buffers for overlays, kept here because resident BSS shares memory with
   the startup code and costs nothing */
unsigned char upool_loaded;
unsigned upool_len;

/* screen lines (also used by shift_from in lowlevel.s) */
unsigned ls[MAXL + 1];
unsigned char lf[MAXL + 1], lh[MAXL], la[MAXL], ly[MAXL + 1];
unsigned char lp[MAXL + 1];             /* paragraph attributes at line start */
unsigned char lq[MAXL], lns[MAXL];      /* paragraph attributes of the line, spaces */
unsigned lw[MAXL];                      /* right edge of the line's text */
unsigned char text_top = 12;            /* first text row (below the ruler) */
unsigned lmargin = 0, rmargin = 512;    /* document margins, pixels from LEFTX */
unsigned dtabs[8];                      /* tab stops, 0 ends */
char doc_head[40], doc_foot[40];        /* printed header and footer */
unsigned char prt_type, prt_slot = 1, prt_quality, prt_pnum = 1;
unsigned char def_fmt;                 /* format of new documents (preferences) */
unsigned char nl, below_dirty, layout_ok;
unsigned char ldirty[MAXL];

/* What is on the screen: one record per displayed line. A dirty line whose
   text is already on the screen somewhere (it only moved) is copied with
   scr_copy instead of being drawn again. */
#define ND (MAXL + 2)
static unsigned ds[ND], de[ND];         /* text range [ds, de) */
static unsigned char dy[ND], dh[ND], df[ND], dq[ND], dv[ND];

static void disp_clear(void)
{
    memset(dv, 0, ND);
}

/* forget the records showing screen rows y0..y1-1 */
static void __fastcall__ disp_rows(unsigned char y0, unsigned char y1)
{
    unsigned char d;
    for (d = 0; d < ND; ++d)
        if (dv[d] && dy[d] < y1 && dy[d] + dh[d] > y0) dv[d] = 0;
}

/* the text changed at p (old coordinates) by delta characters */
#pragma code-name (push, "LC")
void __fastcall__ disp_shift(unsigned p, int delta)
{
    unsigned char d;
    unsigned q = delta < 0 ? p - delta : p;     /* end of the changed range */
    for (d = 0; d < ND; ++d) {
        if (!dv[d]) continue;
        if (ds[d] <= q && de[d] >= p) dv[d] = 0;
        else if (ds[d] > p) { ds[d] += delta; de[d] += delta; }
    }
}
#pragma code-name (pop)

static unsigned caretx;
static unsigned char carety0, carety1, caret_on;
static unsigned goal_x;

#define GAPLEN (ge - gs)

/* ---------------------------------------------------------------------- */
/* Gap buffer                                                              */

unsigned __fastcall__ phys(unsigned p)
{
    return TBASE + (p < gs ? p : p + GAPLEN);
}

unsigned char __fastcall__ tch(unsigned p)
{
    return aux_peek(phys(p));
}

void __fastcall__ fetch(unsigned p, unsigned char *buf, unsigned n)
{
    unsigned a;
    if (p < gs) {
        a = gs - p;
        if (a >= n) { aux_get(buf, TBASE + p, n); return; }
        aux_get(buf, TBASE + p, a);
        buf += a;
        n -= a;
        p = gs;
    }
    aux_get(buf, TBASE + p + GAPLEN, n);
}

static void __fastcall__ move_gap(unsigned p)
{
    unsigned n;
    if (p < gs) {
        n = gs - p;
        aux_move(TBASE + ge - n, TBASE + p, n);
        gs = p;
        ge -= n;
    } else if (p > gs) {
        n = p - gs;
        aux_move(TBASE + gs, TBASE + ge, n);
        gs += n;
        ge += n;
    }
}

/* format checkpoints: format in effect at k*1024, k = 1..31 */
static unsigned char fcp[32], fcpv[32];

static void __fastcall__ fcp_inval(unsigned p)
{
    unsigned char k = (unsigned char)(p >> 10) + 1;
    if (k < 32) memset(fcpv + k, 0, 32 - k);
}

unsigned char __fastcall__ raw_insert(unsigned p, const unsigned char *s, unsigned n)
{
    if (n > CAP - tlen) return 0;
    move_gap(p);
    aux_put(TBASE + gs, s, n);
    gs += n;
    tlen += n;
    modified = 1;
    fcp_inval(p);
    return 1;
}

void __fastcall__ raw_delete(unsigned p, unsigned n)
{
    move_gap(p);
    ge += n;
    tlen -= n;
    modified = 1;
    fcp_inval(p);
}

void text_clear(void)
{
    gs = 1;                             /* one base format marker */
    ge = CAP;
    tlen = 1;
    aux_poke(TBASE, 0x80 | def_fmt);
    cur = anc = 1;
    top = 0;
    topf = topp = tf = 0;
    modified = 0;
    memset(fcpv, 0, 32);
    layout_ok = 0;
    disp_clear();
    doc_head[0] = doc_foot[0] = 0;
    lmargin = 0;                        /* default ruler: 6.4 inches */
    rmargin = 512;
    memset(dtabs, 0, sizeof(dtabs));
}

static unsigned char __fastcall__ fmt_scan(unsigned p, unsigned lo)
{
    unsigned char k, i;
    while (p > lo) {
        k = (p - lo) > 128 ? 128 : (unsigned char)(p - lo);
        p -= k;
        fetch(p, TB, k);
        for (i = k; i; ) {
            --i;
            if ((TB[i] & 0x80) && (TB[i] & 3) != 3) return TB[i] & 0x7F;
        }
    }
    return 0xFF;
}

unsigned char __fastcall__ fmt_at(unsigned p)
{
    unsigned char k, f;
    unsigned c;
    k = (unsigned char)(p >> 10);
    c = (unsigned)k << 10;
    f = fmt_scan(p, c);
    if (f != 0xFF) return f;
    if (!k) return 0;
    if (fcpv[k]) return fcp[k];
    f = fmt_at(c);
    fcp[k] = f;
    fcpv[k] = 1;
    return f;
}

/* ---------------------------------------------------------------------- */
/* Layout                                                                  */

unsigned L_next;
unsigned char L_nf, L_np, L_h, L_a;

/* lay out one line starting at p (format in gfmt, paragraph in lm_pa).
   Results in L_*, lm_pa (the line's paragraph), lm_w, lm_ns. */
static unsigned __fastcall__ lay(unsigned p)
{
    unsigned n = tlen - p;
    unsigned char h;
    if (n > 255) n = 255;
    fetch(p, TB, n);
    lm_n = (unsigned char)n;
    gx = LEFTX + lmargin;
    line_engine(0);
    if (lm_cr == 2 && n < 255) L_next = NOLINE;
    else L_next = p + lm_i;
    L_nf = lm_nf;
    L_np = lm_np;
    L_a = lm_asc;
    h = lm_asc + lm_desc + 1;
    L_h = h + ((lm_pa & 0x30) == 0x10 ? h >> 1 : (lm_pa & 0x30) == 0x20 ? h : 0);
    return L_next;
}

/* store the results of lay() as line i */
static void __fastcall__ lay_store(unsigned char i)
{
    lh[i] = L_h;
    la[i] = L_a;
    lq[i] = lm_pa;
    lw[i] = lm_w;
    lns[i] = lm_ns;
    ls[i + 1] = L_next;
    lf[i + 1] = L_nf;
    lp[i + 1] = L_np;
}

static void layout(void)
{
    unsigned char i = 0, y = text_top;
    ls[0] = top;
    lf[0] = topf;
    lp[0] = topp;
    for (;;) {
        ly[i] = y;
        gfmt = lf[i];
        lm_pa = lp[i];
        lay(ls[i]);
        lay_store(i);
        y += L_h;
        ++i;
        if (y >= BOTY || L_next == NOLINE || i == MAXL) break;
    }
    nl = i;
    ly[nl] = y > BOTY ? BOTY : y;
    layout_ok = 1;
}

void set_all_dirty(void)
{
    memset(ldirty, 1, MAXL);
    below_dirty = 1;
}

void text_moved(void)
{
    layout_ok = 0;
}

void invalidate_layout(void)
{
    disp_clear();
    layout_ok = 0;
}

static unsigned char __fastcall__ line_of(unsigned p)
{
    unsigned char i;
    if (p < top) return 0xFF;
    for (i = 0; i < nl; ++i)
        if (ls[i + 1] == NOLINE || p < ls[i + 1]) return i;
    return 0xFF;
}

static unsigned __fastcall__ line_end(unsigned char i)
{
    return ls[i + 1] == NOLINE ? tlen : ls[i + 1];
}

unsigned __fastcall__ para_start(unsigned p)
{
    unsigned char k;
    while (p) {
        k = p > 128 ? 128 : (unsigned char)p;
        fetch(p - k, TB, k);
        while (k) {
            if (TB[k - 1] == CR) return p;
            --k;
            --p;
        }
    }
    return 0;
}

/* start of the line containing p; its format in L_nf, paragraph in L_np */
static unsigned __fastcall__ line_start_of(unsigned p)
{
    unsigned s, n;
    unsigned char f, a = 0;
    s = para_start(p);
    f = fmt_at(s);
    for (;;) {
        gfmt = f;
        lm_pa = a;
        n = lay(s);
        if (n == NOLINE || n > p) break;
        s = n;
        f = L_nf;
        a = L_np;
    }
    L_nf = f;
    L_np = a;
    return s;
}

/* back k lines from line start s; format in L_nf */
static unsigned ring[MAXL + 2];
static unsigned char ringf[MAXL + 2], ringp[MAXL + 2];
static unsigned __fastcall__ back_lines(unsigned s, unsigned char k)
{
    unsigned ps, q;
    unsigned char m, h, f, a;
    while (k && s) {
        ps = para_start(s - 1);
        f = fmt_at(ps);
        a = 0;
        m = h = 0;
        q = ps;
        while (q != NOLINE && q < s) {
            ring[h] = q;
            ringf[h] = f;
            ringp[h] = a;
            if (++h > k) h = 0;
            ++m;
            gfmt = f;
            lm_pa = a;
            q = lay(q);
            f = L_nf;
            a = L_np;
        }
        if (m >= k) {
            h = (unsigned char)((h + 1) % (k + 1));
            L_nf = ringf[h];
            L_np = ringp[h];
            return ring[h];
        }
        k -= m;
        s = ps;
    }
    L_nf = fmt_at(s);
    L_np = 0;
    return s;
}

/* pen, format, paragraph and justification for drawing line i */
static void __fastcall__ line_setup(unsigned char i)
{
    unsigned slack = 0;
    unsigned char ns;
    gfmt = lf[i];
    lm_pa = lp[i];
    lm_jn = 0;
    gx = LEFTX + lmargin;
    if (lw[i] < LEFTX + rmargin) slack = LEFTX + rmargin - lw[i];
    switch (lq[i] & 0x0C) {
    case 0x04: gx += slack >> 1; break;
    case 0x08: gx += slack; break;
    case 0x0C:
        ns = lns[i];
        if (ns && slack < 256) {
            lm_jn = ns;
            lm_jx = (unsigned char)slack / ns;
            lm_jr = (unsigned char)slack % ns;
        }
    }
}

/* ---------------------------------------------------------------------- */
/* Rendering                                                               */

unsigned sel_lo(void) { return cur < anc ? cur : anc; }
unsigned sel_hi(void) { return cur < anc ? anc : cur; }

/* line i is now on the screen (sel: it shows part of the selection) */
#pragma code-name (push, "LC")
static void __fastcall__ disp_note(unsigned char i, unsigned char sel)
{
    unsigned char d, y = ly[i];
    disp_rows(y, y + lh[i]);
    if (sel) return;
    for (d = 0; d < ND && dv[d]; ++d) ;
    if (d == ND) return;
    ds[d] = ls[i];
    de[d] = line_end(i);
    dy[d] = y;
    dh[d] = lh[i];
    df[d] = lf[i];
    dq[d] = lq[i];
    dv[d] = 1;
}
#pragma code-name (pop)

/* does line i show part of the selection? */
static unsigned char __fastcall__ sel_in(unsigned char i)
{
    unsigned s0 = sel_lo(), s1 = sel_hi();
    return s0 != s1 && s0 <= line_end(i) && s1 > ls[i];
}

/* try to put line i on the screen by copying it from where it is shown */
static unsigned char __fastcall__ disp_copy(unsigned char i)
{
    unsigned char d, y = ly[i], h = lh[i];
    unsigned e;
    if (y + h > BOTY || sel_in(i)) return 0;
    e = line_end(i);
    for (d = 0; d < ND; ++d) {
        if (dv[d] && ds[d] == ls[i] && de[d] == e && dh[d] == h && df[d] == lf[i] && dq[d] == lq[i]) {
            if (dy[d] != y) {
                caret_hide();
                ptr_hide();
                scr_copy(dy[d], y, h, TC0, TC1);
                dv[d] = 0;
                disp_rows(y, y + h);
                dy[d] = y;
                dv[d] = 1;
            }
            ldirty[i] = 0;
            return 1;
        }
    }
    return 0;
}

/* render_line and edited run from the language card to save main memory */
#pragma code-name (push, "LC")
static void __fastcall__ render_line(unsigned char i)
{
    unsigned p, e, s0, s1;
    unsigned char n, rows;
    p = ls[i];
    e = line_end(i);
    n = (unsigned char)(e - p);
    fetch(p, TB, n);
    lb_clear(lh[i]);
    line_setup(i);
    gbase = la[i] - 1;
    lm_n = n;
    lm_s0 = lm_s1 = 0xFF;
    lm_x0 = lm_x1 = 0xFFFF;
    s0 = sel_lo();
    s1 = sel_hi();
    if (s0 != s1 && s0 <= e && s1 > p) {
        if (s0 <= p) lm_x0 = LEFTX - 2; else lm_s0 = (unsigned char)(s0 - p);
        if (s1 < e || (s1 == e && ls[i + 1] == NOLINE)) lm_s1 = (unsigned char)(s1 - p);
        else lm_x1 = LEFTX + rmargin + 4;
    }
    line_engine(3);
    if (n && TB[n - 1] == 0x0C)         /* page break: a dotted line */
        lb_fill(LEFTX, LEFTX + rmargin, lh[i] >> 1, lh[i] >> 1, LB_GRAY);
    if (lm_x0 != 0xFFFF && lm_x1 != 0xFFFF && lm_x1 > lm_x0)
        lb_fill(lm_x0, lm_x1, 0, lh[i] - 1, LB_XOR);
    rows = lh[i];
    if (ly[i] + rows > BOTY) rows = BOTY - ly[i];
    present(ly[i], rows, TC0, TC1);
    ldirty[i] = 0;
    disp_note(i, lm_x0 != 0xFFFF || lm_s0 != 0xFF || lm_s1 != 0xFF);
}
#pragma code-name (pop)

void __fastcall__ clear_rows(unsigned char y, unsigned char y1, unsigned char c0, unsigned char c1)
{
    unsigned char n;
    while (y < y1) {
        n = y1 - y > 32 ? 32 : y1 - y;
        lb_clear(n);
        present(y, n, c0, c1);
        y += n;
    }
}

/* scroll bar: data-driven pieces (mode, x0, x1, r0, r1 relative to SBX) */
static const unsigned char sb_up[] = {
    1, 0, 0, 2, 0, 0, 10,      1, 0, 0, 21, 0, 10, 10,
    1, 9, 0, 12, 0, 2, 2,      1, 8, 0, 13, 0, 3, 3,
    1, 7, 0, 14, 0, 4, 4,      1, 6, 0, 15, 0, 5, 5,
    1, 5, 0, 16, 0, 6, 6,      1, 8, 0, 13, 0, 7, 8,
    0xFF
};
static const unsigned char sb_dn[] = {
    1, 0, 0, 2, 0, 0, 10,      1, 0, 0, 21, 0, 0, 0,
    1, 8, 0, 13, 0, 2, 3,      1, 5, 0, 16, 0, 4, 4,
    1, 6, 0, 15, 0, 5, 5,      1, 7, 0, 14, 0, 6, 6,
    1, 8, 0, 13, 0, 7, 7,      1, 9, 0, 12, 0, 8, 8,
    0xFF
};
static const unsigned char sb_thumb[] = {
    1, 0, 0, 2, 0, 0, 10,      1, 0, 0, 21, 0, 0, 0,
    1, 0, 0, 21, 0, 10, 10,    0xFF
};
static const unsigned char sb_track[] = {
    1, 0, 0, 2, 0, 0, 31,      3, 2, 0, 21, 0, 0, 31,
    0xFF
};

static unsigned char thumb_y;
#define SBTRACK (BOTY - MENUH - 33)

static void __fastcall__ sb_piece(const unsigned char *cmds, unsigned char y, unsigned char n)
{
    lb_clear(n);
    lbx = SBX;
    lb_cmds(cmds);
    present(y, n, 77, 79);
}

static unsigned char sb_full;          /* whole bar on screen */

/* draws the whole bar after redraw_all, otherwise only moves the thumb */
static void draw_scrollbar(void)
{
    unsigned char y, n, ny;
    unsigned t = 0;
    if (tlen > 1) t = (top >> 6) * SBTRACK / ((tlen >> 6) + 1);
    ny = MENUH + 11 + (unsigned char)t;
    if (sb_full) {
        if (ny == thumb_y) return;
        y = thumb_y - ((thumb_y - MENUH) & 1);     /* keep the gray pattern in phase */
        sb_piece(sb_track, y, 12);
        if (y < MENUH + 11) sb_piece(sb_up, MENUH, 11);
        if (y + 12 > BOTY - 11) sb_piece(sb_dn, BOTY - 11, 11);
    } else {
        for (y = MENUH; y < BOTY; y += n) {
            n = BOTY - y > 32 ? 32 : BOTY - y;
            sb_piece(sb_track, y, n);
        }
        sb_piece(sb_up, MENUH, 11);
        sb_piece(sb_dn, BOTY - 11, 11);
        sb_full = 1;
    }
    thumb_y = ny;
    sb_piece(sb_thumb, thumb_y, 11);
}

/* ---------------------------------------------------------------------- */
/* Ruler                                                                   */

unsigned char ruler_on, ruler_dirty, ruler_pa;

/* margins and tabs -> line engine */
void ruler_apply(void)
{
    unsigned char i;
    wrapx = LEFTX + rmargin;
    tabbase = LEFTX + lmargin;
    for (i = 0; i < 8; ++i) tabs[i] = dtabs[i] ? LEFTX + dtabs[i] : 0;
    text_top = ruler_on ? MENUH + RULERH + 1 : MENUH + 1;
}

/* paragraph attributes at the caret */
unsigned char caret_pa(void)
{
    unsigned char i;
    if (!layout_ok) return 0;
    i = line_of(cur);
    return i == 0xFF ? 0 : lq[i];
}

/* icon box k (0-3 alignment, 4-6 spacing) */
static void __fastcall__ ricon(unsigned char k, unsigned char sel)
{
    unsigned x = LEFTX + k * 22 + (k > 3 ? 16 : 0);
    unsigned char r, w, o, step;
    frame_rect(x, x + 19, 16, 25);
    step = k < 4 ? 2 : k - 2;               /* line gap: 2, 2, 3, 4 */
    for (r = 18; r < 24; r += step) {
        w = (k < 4 && r == 20) ? 8 : 12;
        o = 0;
        if (k == 1) o = (12 - w) >> 1;
        if (k == 2) o = 12 - w;
        if (k == 3) w = 12;
        lb_fill(x + 4 + o, x + 4 + o + w, r, r, LB_SET);
    }
    if (sel) lb_fill(x + 2, x + 18, 17, 24, LB_XOR);
}

#pragma code-name (push, "LC")
/* a small marker hanging below the scale at x: 0 margin (triangle), 1 tab */
static void __fastcall__ rmark(unsigned x, unsigned char tab)
{
    unsigned char r;
    if (tab) {
        lb_fill(x, x + 2, 11, 14, LB_SET);
        lb_fill(x - 2, x + 4, 14, 15, LB_SET);
        return;
    }
    for (r = 0; r < 4; ++r) lb_fill(x - r, x + r + 2, 11 + r, 11 + r, LB_SET);
}

void draw_ruler(void)
{
    unsigned x;
    unsigned char i, pa;
    static char num[2];
    ruler_dirty = 0;
    if (!ruler_on) return;
    pa = ruler_pa = caret_pa();
    lb_clear(RULERH + 1);
    lb_fill(LEFTX, LEFTX + 513, 10, 10, LB_SET);
    for (x = 0, i = 0; x <= 512; x += 10, ++i) {
        lb_fill(LEFTX + x, LEFTX + x + 1, (i & 7) == 0 ? 5 : (i & 3) == 0 ? 7 : 9, 9, LB_SET);
        if ((i & 7) == 0 && x) {
            num[0] = '0' + (i >> 3);
            ui_text(LEFTX + x + 3, 6, 0, num);
        }
    }
    rmark(LEFTX + lmargin, 0);
    rmark(LEFTX + rmargin, 0);
    for (i = 0; i < 8 && dtabs[i]; ++i) rmark(LEFTX + dtabs[i], 1);
    for (i = 0; i < 4; ++i) ricon(i, ((pa >> 2) & 3) == i);
    for (i = 0; i < 3; ++i) ricon(4 + i, ((pa >> 4) & 3) == i);
    lb_fill(0, 539, RULERH, RULERH, LB_SET);
    ptr_hide();
    present(MENUH, RULERH + 1, 0, 76);
}
#pragma code-name (pop)

void redraw_all(void)
{
    ptr_hide();
    caret_on = 0;
    ruler_apply();
    draw_menubar();
    clear_rows(MENUH, BOTY, 0, 76);
    draw_ruler();
    disp_clear();
    sb_full = 0;
    draw_scrollbar();
    layout_ok = 0;
    set_all_dirty();
}

/* ---------------------------------------------------------------------- */
/* Caret                                                                   */

static unsigned __fastcall__ x_in_line(unsigned char i, unsigned p)
{
    unsigned q = ls[i];
    lm_n = (unsigned char)(p - q);
    fetch(q, TB, lm_n);
    line_setup(i);
    line_engine(1);
    return gx;
}

static void caret_toggle(void)
{
    if (carety1 <= carety0) return;
    ptr_hide();
    caret_xor(caretx, carety0, carety1);
    caret_on ^= 1;
}

void caret_hide(void)
{
    if (caret_on) caret_toggle();
}

static void locate_caret(void)
{
    unsigned char i = line_of(cur);
    carety0 = carety1 = 0;
    if (i == 0xFF) return;
    caretx = x_in_line(i, cur) - 1;
    carety0 = ly[i];
    carety1 = ly[i] + lh[i] - 2;
    if (carety1 >= BOTY) carety1 = BOTY - 1;
}

/* ---------------------------------------------------------------------- */
/* Screen update                                                           */

/* make the caret line visible above screen row ybot */
void __fastcall__ scroll_into(unsigned char ybot)
{
    unsigned char i;
    if (!layout_ok) layout();
    if (cur >= top) {
        i = line_of(cur);
        if (i != 0xFF && (ly[i] + lh[i] <= ybot || i == 0)) return;
    }
    top = back_lines(line_start_of(cur), ybot < BOTY ? 2 : 4);
    topf = L_nf; topp = L_np;
    layout();
    set_all_dirty();
}

void scroll_to_caret(void)
{
    scroll_into(BOTY);
}

/* copy line i if it moves in direction down (0 = up or not at all) */
static void __fastcall__ disp_copy_dir(unsigned char i, unsigned char down)
{
    unsigned char d;
    for (d = 0; d < ND; ++d)
        if (dv[d] && ds[d] == ls[i]) {
            if ((dy[d] < ly[i]) == down) disp_copy(i);
            return;
        }
}

static unsigned char __fastcall__ update(unsigned char abortable)
{
    unsigned char i;
    if (!layout_ok) { layout(); set_all_dirty(); }
    /* lines that only moved: copy them, moving up top-down and moving down
       bottom-up so that no copy destroys another one's source */
    for (i = 0; i < nl; ++i) if (ldirty[i]) disp_copy_dir(i, 0);
    for (i = nl; i--; ) if (ldirty[i]) disp_copy_dir(i, 1);
    for (i = 0; i < nl; ++i) {
        if (ldirty[i]) {
            if (abortable && (PEEK(KBD) & 0x80)) return 0;
            caret_hide();
            ptr_hide();
            render_line(i);
        }
    }
    if (below_dirty) {
        ptr_hide();
        clear_rows(ly[nl], BOTY, TC0, TC1);
        disp_rows(ly[nl], BOTY);
        below_dirty = 0;
    }
    return 1;
}

void refresh(void)
{
    update(0);
}

/* after an edit at p (old coordinates) changing the length by delta */
unsigned olds[MAXL + 1];
unsigned char oldf[MAXL + 1], oldp[MAXL + 1];
unsigned ed_p;
int ed_delta;

/* New line j0 starts where old line k started (lines were added or
   removed above it): the old layout from line k on is reused, shifted.
   (lowlevel.s) */
unsigned char __fastcall__ shift_from(unsigned char j0);
static void __fastcall__ edited(unsigned p, int delta)
{
    unsigned char r, i, j;
    unsigned q;
    disp_shift(p, delta);
    if (!layout_ok || p < top) { layout_ok = 0; scroll_to_caret(); return; }
    r = line_of(p);
    if (r == 0xFF) { layout_ok = 0; return; }
    if (r == 0 && top > 0) {
        q = line_start_of(top);
        if (q != top) { top = q; topf = L_nf; topp = L_np; layout_ok = 0; return; }
    }
    memcpy(olds, ls, sizeof(olds));
    memcpy(oldf, lf, sizeof(oldf));
    memcpy(oldp, lp, sizeof(oldp));
    for (i = r ? r - 1 : 0; i < nl; ++i) {
        gfmt = lf[i];
        lm_pa = lp[i];
        lay(ls[i]);
        ldirty[i] = 1;
        if (L_h != lh[i]) { layout_ok = 0; return; }   /* heights changed */
        lay_store(i);
        if (L_next == NOLINE) {
            if (olds[i + 1] != NOLINE) layout_ok = 0;
            return;
        }
        q = olds[i + 1];
        if (q != NOLINE && q > p) q += delta;
        if (L_next > p + (delta > 0 ? delta : 0)) {
            if (L_next == q && L_nf == oldf[i + 1] && L_np == oldp[i + 1]) {
                for (j = i + 2; j <= nl; ++j)
                    if (olds[j] != NOLINE && olds[j] > p) ls[j] = olds[j] + delta;
                return;
            }
            ed_p = p;
            ed_delta = delta;
            if (shift_from(i + 1)) return;
        }
    }
    layout_ok = 0;
}

/* ---------------------------------------------------------------------- */
/* Editing                                                                 */

void __fastcall__ sel_changed(unsigned olo, unsigned ohi)
{
    unsigned lo = sel_lo(), hi = sel_hi(), a, b;
    unsigned char i;
    if (olo == ohi && lo == hi) return;
    a = olo < lo ? olo : lo;
    b = ohi > hi ? ohi : hi;
    for (i = 0; i < nl; ++i)
        if (ls[i] <= b && line_end(i) + 1 >= a) ldirty[i] = 1;
}

void __fastcall__ delete_range(unsigned p, unsigned n)
{
    if (!n) return;
    raw_delete(p, n);
    if (cur > p) cur = cur >= p + n ? cur - n : p;
    if (anc > p) anc = anc >= p + n ? anc - n : p;
    edited(p, -(int)n);
}

static void __fastcall__ insert_mark(unsigned p, unsigned char f)
{
    unsigned char m = 0x80 | f;
    raw_insert(p, &m, 1);
    edited(p, 1);
}

unsigned char delete_selection(void)
{
    unsigned lo = sel_lo(), hi = sel_hi();
    unsigned char f;
    if (lo == hi) return 0;
    f = fmt_at(hi);
    delete_range(lo, hi - lo);
    cur = anc = lo;
    if (fmt_at(lo) != f) {
        insert_mark(lo, f);
        cur = anc = lo + 1;
    }
    return 1;
}

void __fastcall__ insert_char(unsigned char c)
{
    unsigned p;
    delete_selection();
    p = cur;
    if (tlen >= CAP - 2) { alert("The document is full.", "OK", 0, 0); return; }
    if (fmt_at(p) != tf) { insert_mark(p, tf); ++p; }
    raw_insert(p, &c, 1);
    edited(p, 1);
    cur = anc = p + 1;
}

unsigned __fastcall__ skip_fwd(unsigned p)
{
    while (p < tlen && (tch(p) & 0x80)) ++p;
    return p;
}

static unsigned __fastcall__ char_left(unsigned p)
{
    while (p > 1) {
        --p;
        if (!(tch(p) & 0x80)) return p;
    }
    return 1;
}

static unsigned __fastcall__ char_right(unsigned p)
{
    p = skip_fwd(p);
    if (p < tlen) ++p;
    return skip_fwd(p);
}

unsigned char __fastcall__ isword(unsigned char c)
{
    unsigned char l = c | 0x20;
    return (l >= 'a' && l <= 'z') || (c >= '0' && c <= '9') || c == '\'';
}

/* ---------------------------------------------------------------------- */
/* Mouse in text                                                           */

static unsigned __fastcall__ pos_at_xy(unsigned x, unsigned char y)
{
    unsigned char i;
    unsigned p;
    if (!layout_ok) layout();
    for (i = 0; i + 1 < nl; ++i)
        if (y < ly[i] + lh[i]) break;
    p = ls[i];
    lm_n = (unsigned char)(line_end(i) - p);
    fetch(p, TB, lm_n);
    line_setup(i);
    lm_tx = x;
    line_engine(2);
    p += lm_i;
    if (lm_i == lm_n && ls[i + 1] != NOLINE && p == ls[i + 1] && p > ls[i]) --p;
    return p ? p : 1;
}

/* ---------------------------------------------------------------------- */
/* Scrolling                                                               */

static void __fastcall__ scroll_lines(signed char n)
{
    if (!layout_ok) layout();
    if (n > 0) {
        while (n-- && ls[1] != NOLINE && nl > 1) {
            top = ls[1];
            topf = lf[1];
            topp = lp[1];
            layout();
        }
    } else if (top > 0) {
        top = back_lines(top, -n);
        topf = L_nf; topp = L_np;
        layout();
    }
    caret_on = 0;
    set_all_dirty();
}

static void __fastcall__ page(signed char dir)
{
    unsigned char keep;
    if (!layout_ok) layout();
    keep = nl > 2 ? nl - 2 : 1;
    if (dir > 0) {
        if (ls[keep] == NOLINE) { cur = anc = tlen; return; }
        top = ls[keep];
        topf = lf[keep];
        topp = lp[keep];
    } else {
        if (!top) { cur = anc = 1; return; }
        top = back_lines(top, keep);
        topf = L_nf; topp = L_np;
    }
    layout();
    if (line_of(cur) == 0xFF) cur = anc = skip_fwd(ls[0] ? ls[0] : 1);
    set_all_dirty();
}

static void __fastcall__ move_vert(signed char d)
{
    unsigned char i;
    if (!layout_ok) layout();
    i = line_of(cur);
    if (i == 0xFF) { scroll_to_caret(); i = line_of(cur); }
    if (d < 0) {
        if (i == 0) {
            if (top <= 1) { cur = anc = 1; return; }
            scroll_lines(-1);
            i = 1;
        }
        cur = anc = pos_at_xy(goal_x, ly[i - 1]);
    } else {
        if (ls[i + 1] == NOLINE) { cur = anc = tlen; return; }
        if (i + 1 >= nl || ly[i + 1] + lh[i + 1] > BOTY) {
            scroll_lines(1);
            i = line_of(cur);
            if (i == 0xFF || i + 1 >= nl) { scroll_to_caret(); return; }
        }
        cur = anc = pos_at_xy(goal_x, ly[i + 1]);
    }
}

void __fastcall__ after_move(unsigned olo, unsigned ohi, unsigned char keepgoal)
{
    unsigned char i;
    caret_hide();
    scroll_to_caret();
    sel_changed(olo, ohi);
    if (!keepgoal) {
        i = line_of(cur);
        if (i != 0xFF) goal_x = x_in_line(i, cur);
    }
    tf = fmt_at(cur);
}

#include "ui.inc"

static void __fastcall__ do_key(unsigned char k)
{
    unsigned olo = sel_lo(), ohi = sel_hi();
    unsigned char keep = 0;
    unsigned q;
    caret_hide();
    if (oa) { q = oa_special(k); command(q ? q : menus(2)); return; }
    switch (k) {
    case K_LEFT: cur = anc = (olo != ohi) ? olo : char_left(cur); break;
    case K_RIGHT: cur = anc = (olo != ohi) ? ohi : char_right(cur); break;
    case K_UP: move_vert(-1); keep = 1; break;
    case K_DOWN: move_vert(1); keep = 1; break;
    case K_ESC: command(menus(1)); return;
    case 7: command(CM_GLOSSARY); return;          /* Control-G */
    case K_DEL:
        if (!delete_selection()) {
            q = cur;
            while (q > 1 && (tch(q - 1) & 0x80)) --q;
            if (q > 1) delete_range(q - 1, 1);
        }
        break;
    case K_RET: insert_char(CR); break;
    default:
        if (k == K_TAB || (k >= 0x20 && k < 0x7F)) insert_char(k);
        break;
    }
    after_move(olo, ohi, keep);
}

#define DCLICK 500              /* idle loops of ~800 cycles: about 0.4 s */
static unsigned dclick_t;
static unsigned dclick_p;

static unsigned __fastcall__ word_start(unsigned q)
{
    while (q > 1 && isword(tch(q - 1))) --q;
    return q;
}

static unsigned __fastcall__ word_end(unsigned q)
{
    while (q < tlen && isword(tch(q))) ++q;
    return q;
}

/* click: place the caret; Apple-click extends; double click selects a word;
   dragging extends the selection (by words after a double click) */
static void click_text(void)
{
    unsigned p, olo = sel_lo(), ohi = sel_hi(), lastp, w0, w1;
    unsigned char words = 0;
    p = pos_at_xy(mx, my);
    if (dclick_t && p == dclick_p) {    /* double click: select a word */
        words = 1;
        anc = w0 = word_start(p);
        cur = w1 = word_end(p);
        dclick_t = 0;
    } else if ((PEEK(OAPPLE) | PEEK(SAPPLE)) & 0x80) {
        cur = p;                        /* Apple-click extends the selection */
        dclick_t = 0;
    } else {
        cur = anc = p;
        dclick_t = DCLICK;
    }
    dclick_p = lastp = p;
    sel_changed(olo, ohi);
    update(0);
    while (mbtn) {                      /* drag to extend the selection */
        mouse_poll();
        olo = sel_lo();
        ohi = sel_hi();
        if (my < text_top && top > 0) scroll_lines(-1);
        else if (my >= BOTY - 2) scroll_lines(1);
        p = pos_at_xy(mx, my);
        if (p != lastp) {
            lastp = p;
            dclick_t = 0;
            if (!words) cur = p;
            else if (p < w0) { anc = w1; cur = word_start(p); }
            else { anc = w0; cur = word_end(p); }
            sel_changed(olo, ohi);
            update(0);
        }
        ptr_hide();
        ptr_show();
    }
    after_move(olo, ohi, 0);
}

static void click_scrollbar(void)
{
    unsigned char y = my;
    if (y < MENUH + 11 || y >= BOTY - 11) {
        do {
            scroll_lines(y < MENUH + 11 ? -1 : 1);
            update(0);
            draw_scrollbar();
            mouse_poll();
        } while (mbtn);
    } else if (y < thumb_y) {
        page(-1);
    } else if (y >= thumb_y + 11) {
        page(1);
    } else {
        wait_up();
        y = my < MENUH + 11 ? MENUH + 11 : (my > BOTY - 22 ? BOTY - 22 : my);
        top = line_start_of(((tlen >> 6) * (y - MENUH - 11) / SBTRACK) << 6);
        topf = L_nf; topp = L_np;
        layout();
        set_all_dirty();
        if (line_of(cur) == 0xFF) cur = anc = skip_fwd(top ? top : 1);
    }
    draw_scrollbar();
    wait_up();
}

/* ---------------------------------------------------------------------- */
/* Event loop                                                              */

static unsigned blinkcnt;
static unsigned char sbshown;

unsigned char wait_event(void)
{
    unsigned ox;
    unsigned char oy;
    for (;;) {
        if (PEEK(KBD) & 0x80) {
            key = PEEK(KBD) & 0x7F;
            oa = (PEEK(OAPPLE) | PEEK(SAPPLE)) & 0x80;
            POKE(KBDSTRB, 0);
            return 1;
        }
        ox = mx;
        oy = my;
        mouse_poll();
        if (mx != ox || my != oy || !ptr_on) { ptr_hide(); pshape = 0; ptr_show(); }
        if (mbtn) return 2;
    }
}

/* wait for the button to go up; the pointer keeps following the mouse */
void wait_up(void)
{
    unsigned ox;
    unsigned char oy;
    while (mbtn) {
        ox = mx;
        oy = my;
        mouse_poll();
        if (mx != ox || my != oy) { ptr_hide(); ptr_show(); }
    }
}

void beep(void)
{
    unsigned char i, j;
    for (i = 0; i < 60; ++i) {
        PEEK(0xC030);
        for (j = 0; j < 30; ++j) ;
    }
}

static void idle_draw(void)
{
    if (!update(1)) return;
    if (ruler_on && (ruler_dirty || caret_pa() != ruler_pa)) draw_ruler();
    if (!sbshown) { ptr_hide(); draw_scrollbar(); sbshown = 1; }
    if (!caret_on && cur == anc) {
        locate_caret();
        caret_toggle();
        blinkcnt = 0;
    }
}

static void handle_mouse(void)
{
    unsigned ox = mx;
    unsigned char oy = my, s;
    mouse_poll();
    s = (my >= text_top && mx < SBX);
    if (mx != ox || my != oy || !ptr_on || s != pshape) {
        ptr_hide();
        pshape = s;
        ptr_show();
    }
    if (!mbtn) return;
    caret_hide();
    if (my < MENUH) command(menus(0));
    else if (mx >= SBX) click_scrollbar();
    else if (my < text_top) { if (load_ovl(OV_MENU)) ruler_click(); }
    else click_text();
    sbshown = 0;
}

void editor_loop(void)
{
    for (;;) {
        idle_draw();
        if (PEEK(KBD) & 0x80) {
            key = PEEK(KBD) & 0x7F;
            oa = (PEEK(OAPPLE) | PEEK(SAPPLE)) & 0x80;
            POKE(KBDSTRB, 0);
            do_key(key);
            sbshown = 0;
        } else {
            handle_mouse();
            if (dclick_t) --dclick_t;
            if (++blinkcnt >= 550 && cur == anc) {
                blinkcnt = 0;
                locate_caret();
                caret_toggle();
            }
        }
        if (quit_now) return;
    }
}

