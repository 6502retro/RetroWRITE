/*
** RetroWrite II - overlay 10: read an ODT document's XML into RetroWrite
**
** Overlay 9 has unpacked styles.xml to RW2.XML1 and content.xml to
** RW2.XML2.  styles.xml gives the default and named styles and the page
** header and footer; content.xml gives the automatic styles and the text.
**
** Kept: paragraphs, Sans/Serif/Mono (by font name), 9/18/27 point (from the
** point size: up to 17pt is 9, up to 29pt is 18, larger is 27), bold,
** italic, underline, superscript, subscript, alignment, line spacing,
** page breaks, tab stops, the first paragraph's indents as the ruler
** margins, header and footer (with the page number as #).
** Letters outside ASCII become the nearest plain letter; pictures, notes
** and comments are left out.
*/
#pragma code-name (push, "OVLA")
#pragma rodata-name (push, "OVLA")
#pragma data-name (push, "OVLA")
#pragma bss-name (push, "OVLABSS")

#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <apple2enh.h>
#include "rw.h"

/* ---------------------------------------------------------------------- */
/* styles                                                                  */

#define TM_FONT 1
#define TM_SIZE 2
#define TM_POS  4
#define TM_B    8
#define TM_I    16
#define TM_U    32
#define PM_AL   1
#define PM_SP   2
#define PM_ML   8
#define PM_MR   16

struct st {
    unsigned h, par;
    unsigned char tm, font, size, pos, b, it, u;
    unsigned char pm, al, sp, brk;
    unsigned ml, mr;
};
/* the table and buffers sit in the line buffer ($1400-$1D7F), which
   nothing draws into meanwhile */
#define NST 100
#define sty ((struct st *)0x1400)       /* 100 x 19 bytes, to $1B6B */
#define C (*(struct st *)0x1B80)        /* the style being read */
#define D (*(struct st *)0x1BA0)        /* the default paragraph style */
#define T (*(struct st *)0x1BC0)        /* scratch */
#define R (*(struct st *)0x1BE0)        /* the result of resolving a style */
#define stab ((unsigned *)0x1C00)       /* 8 tab stops */
#define fstk ((unsigned char *)0x1C10)  /* 8 formats */
#define obuf ((unsigned char *)0x1C80)  /* 128 */
#define tname ((char *)0x1D00)          /* 32 */
#define aname ((char *)0x1D20)          /* 32 */
#define aval ((char *)0x1D40)           /* 48 */
static unsigned char nst, incs;

static unsigned __fastcall__ hash(const char *s)
{
    unsigned h = 7;
    while (*s) h = h * 31 + (unsigned char)*s++;
    return h ? h : 1;
}

static unsigned char __fastcall__ find(unsigned h)
{
    unsigned char i;
    for (i = nst; i--; )
        if (sty[i].h == h) return i;
    return 0xFF;
}

static void apply(void)                 /* T onto R */
{
    unsigned char m = T.tm;
    if (m & TM_FONT) R.font = T.font;
    if (m & TM_SIZE) R.size = T.size;
    if (m & TM_POS) R.pos = T.pos;
    if (m & TM_B) R.b = T.b;
    if (m & TM_I) R.it = T.it;
    if (m & TM_U) R.u = T.u;
    m = T.pm;
    if (m & PM_AL) R.al = T.al;
    if (m & PM_SP) R.sp = T.sp;
    if (T.brk) R.brk = 1;
    if (m & PM_ML) R.ml = T.ml;
    if (m & PM_MR) R.mr = T.mr;
    R.pm |= m;
}

/* applies style h and its parents, the farthest parent first (no
   recursion: locals are static with -Cl) */
static void __fastcall__ chain(unsigned h)
{
    unsigned char k[6], n = 0, i;
    while (n < 6 && (i = find(h)) != 0xFF) {
        k[n++] = i;
        h = sty[i].par;
        if (!h) break;
    }
    while (n) {
        memcpy(&T, &sty[k[--n]], sizeof(T));
        apply();
    }
}

static unsigned char fmt_of(void)
{
    unsigned char f = R.font;
    if (R.pos) {
        f += F_SIZE;
        if (R.pos == 2) f += F_BOLD;
    } else {
        f += R.size << 2;
        if (R.b) f += F_BOLD;
    }
    if (R.it) f += F_ITAL;
    if (R.u) f += F_UND;
    return f;
}

/* R from a format byte */
static void __fastcall__ from_fmt(unsigned char f)
{
    R.font = f & F_FONT;
    R.size = (f & F_SIZE) >> 2;
    R.pos = R.b = 0;
    if (R.size == 3) { R.pos = (f & F_BOLD) ? 2 : 1; R.size = 0; }
    else if (f & F_BOLD) R.b = 1;
    R.it = (f & F_ITAL) != 0;
    R.u = (f & F_UND) != 0;
}

/* ---------------------------------------------------------------------- */
/* XML input                                                               */

static int fd;
static unsigned ip, il;
static unsigned char eof;

static unsigned char nextc(void)
{
    int n;
    if (ip >= il) {
        n = read(fd, iob, 512);
        if (n <= 0) { eof = 1; return 0; }
        il = n;
        ip = 0;
    }
    return iob[ip++];
}

/* ---------------------------------------------------------------------- */
/* output: the document                                                    */

static unsigned char on, efmt, trunc_;

static void oflush(void)
{
    if (!on) return;
    if (gs + on > CAP - 2) trunc_ = 1;
    else {
        aux_put(TBASE + gs, obuf, on);
        gs += on;
    }
    on = 0;
}

static void __fastcall__ ob(unsigned char c)
{
    obuf[on++] = c;
    if (on == 128) oflush();
}

/* ---------------------------------------------------------------------- */
/* names                                                                   */

enum { E_NONE, E_STYLE, E_DEFSTYLE, E_PPROP, E_TPROP, E_TABSTOP, E_P, E_H, E_SPAN,
       E_S, E_TAB, E_LB, E_LITEM, E_HEADER, E_FOOTER, E_PAGENUM, E_BODY, E_MASTER, E_SKIP };

static const char *const enames[] = {
    "style:style", "style:default-style", "style:paragraph-properties", "style:text-properties",
    "style:tab-stop", "text:p", "text:h", "text:span", "text:s", "text:tab", "text:line-break",
    "text:list-item", "style:header", "style:footer", "text:page-number", "office:body",
    "style:master-page",
    /* left out with everything inside them */
    "text:note", "office:annotation", "text:tracked-changes", "draw:frame", "svg:desc",
    "svg:title", "office:forms", "style:header-left", "style:footer-left", "style:header-first",
    "style:footer-first", "text:sequence-decls", "draw:custom-shape", 0
};

enum { A_NAME, A_SNAME, A_PARENT, A_FAMILY, A_C, A_FSIZE, A_FWEIGHT, A_FSTYLE, A_UNDER, A_TPOS,
       A_FNAME, A_FFAMILY, A_ALIGN, A_LH, A_BREAK, A_ML, A_MR, A_POS };
static const char *const anames[] = {
    "style:name", "text:style-name", "style:parent-style-name", "style:family", "text:c",
    "fo:font-size", "fo:font-weight", "fo:font-style", "style:text-underline-style",
    "style:text-position", "style:font-name", "fo:font-family", "fo:text-align",
    "fo:line-height", "fo:break-before", "fo:margin-left", "fo:margin-right", "style:position", 0
};
static const char *const ents[] = { "amp", "lt", "gt", "quot", "apos", 0 };


static unsigned char __fastcall__ lookup(const char *const *t, const char *s)
{
    unsigned char i;
    for (i = 0; t[i]; ++i)
        if (!strcmp(s, t[i])) return i;
    return 0xFF;
}

static unsigned char __fastcall__ elem(const char *s)
{
    unsigned char n = lookup(enames, s);
    return n == 0xFF ? E_NONE : n < E_SKIP - 1 ? n + 1 : E_SKIP;
}

static unsigned char __fastcall__ is(const char *w)
{
    return !strcmp(aval, w);
}

static unsigned char __fastcall__ has(const char *w)
{
    unsigned char n = (unsigned char)strlen(w);
    const char *s;
    for (s = aval; *s; ++s)
        if (*s == *w && !memcmp(s, w, n)) return 1;
    return 0;
}

/* aval "12.5cm" -> pixels at 80 per inch; pt gets points (font sizes) */
static unsigned pt;
static unsigned length(void)
{
    unsigned ip_ = 0, fr = 0, m = 100;
    const char *s = aval;
    pt = 0;
    if (*s == '-') return 0;
    for (; *s >= '0' && *s <= '9'; ++s) { ip_ += ip_ << 2; ip_ = (ip_ << 1) + (*s - '0'); }
    if (*s == '.')
        for (++s; *s >= '0' && *s <= '9'; ++s) { fr += (*s - '0') * m; m /= 10; }
    switch (*s) {
    case 'i': pt = ip_ * 72 + fr / 14; return ip_ * 80 + fr / 13;
    case 'c': pt = ip_ * 28 + ip_ * 35 / 100 + fr / 35; return ip_ * 31 + (ip_ >> 1) + fr / 32;
    case 'm': pt = (ip_ << 1) + ip_ * 83 / 100; return ip_ + (ip_ << 1) + ip_ * 15 / 100;
    case 'p':
        if (s[1] == 'c') { pt = ip_ * 12; return ip_ * 13 + ip_ / 3; }
    }
    pt = ip_;
    return ip_ + ip_ / 9;
}

/* to the ruler's 1/16 inch */
static unsigned snap(void)
{
    return (length() + 2) / 5 * 5;
}

static const char *const mono_n[] = { "Mono", "Courier", "Consol", 0 };
static const char *const sans_n[] = { "Sans", "Arial", "Helvet", "Calibri", "Verdana", 0 };
static const char *const serif_n[] = { "Serif", "Times", "Roman", "Georgia", "Cambria", "Garamond", 0 };
static unsigned char __fastcall__ any(const char *const *t)
{
    for (; *t; ++t) if (has(*t)) return 1;
    return 0;
}

static unsigned char font_of(void)
{
    if (any(mono_n)) return 2;
    if (any(sans_n)) return 0;
    if (any(serif_n)) return 1;
    return def_fmt & F_FONT;
}

/* ---------------------------------------------------------------------- */
/* parser state                                                            */

static unsigned char dep, skipat, eid, stylexml, inbody, master;
static unsigned char dest;              /* 0 nothing, 1 document, 2 header, 3 footer */
static unsigned char pendf, lastpa, inpara, pend, pstarted, listp, ntabs, tabsdone, firstp;
static unsigned char fsp, fmt;
static unsigned spar, sname, scount;
static unsigned char sfam;              /* 1 paragraph, 2 text */
static char *hfp;
static unsigned char hfn;

/* an attribute of the current element */
static void __fastcall__ attr(unsigned char a)
{
    unsigned char i;
    unsigned v;
    switch (a) {
    case A_NAME: case A_SNAME: sname = hash(aval); return;
    case A_PARENT: spar = hash(aval); return;
    case A_FAMILY: sfam = is("paragraph") ? 1 : is("text") ? 2 : 0; return;
    case A_C: scount = 0; for (i = 0; aval[i]; ++i) scount = scount * 10 + aval[i] - '0'; return;
    }
    if (!incs) return;
    if (eid == E_TPROP) {
        switch (a) {
        case A_FSIZE:
            if (strchr(aval, '%')) return;
            length();
            C.size = pt < 18 ? 0 : pt < 30 ? 1 : 2;
            C.tm |= TM_SIZE;
            break;
        case A_FWEIGHT:
            C.b = is("bold") || (aval[0] >= '6' && aval[0] <= '9');
            C.tm |= TM_B;
            break;
        case A_FSTYLE: C.it = !is("normal"); C.tm |= TM_I; break;
        case A_UNDER: C.u = !is("none"); C.tm |= TM_U; break;
        case A_TPOS:
            C.pos = !memcmp(aval, "super", 5) ? 1 : (!memcmp(aval, "sub", 3) || aval[0] == '-') ? 2 :
                    (aval[0] >= '1' && aval[0] <= '9') ? 1 : 0;
            C.tm |= TM_POS;
            break;
        case A_FNAME: case A_FFAMILY: C.font = font_of(); C.tm |= TM_FONT; break;
        }
    } else if (eid == E_PPROP) {
        switch (a) {
        case A_ALIGN:
            C.al = is("center") ? 1 : (is("end") || is("right")) ? 2 : is("justify") ? 3 : 0;
            C.pm |= PM_AL;
            break;
        case A_LH:
            if (!strchr(aval, '%')) return;
            length();
            C.sp = pt >= 175 ? 2 : pt >= 125 ? 1 : 0;
            C.pm |= PM_SP;
            break;
        case A_BREAK: if (is("page")) C.brk = 1; break;
        case A_ML: C.ml = snap(); C.pm |= PM_ML; break;
        case A_MR: C.mr = snap(); C.pm |= PM_MR; break;
        }
    } else if (eid == E_TABSTOP && a == A_POS && ntabs < 8) {
        v = snap();
        if (v) stab[ntabs++] = v;
    }
}

/* ---------------------------------------------------------------------- */
/* text                                                                    */

static void __fastcall__ put(unsigned char c)
{
    if (dest == 1) {
        if (efmt != fmt) { efmt = fmt; ob(0x80 | fmt); }
        ob(c);
    } else if (dest >= 2 && hfn < 39) {
        if (c == TAB) c = ' ';
        if (c == ' ' && (!hfn || hfp[hfn - 1] == ' ')) return;
        hfp[hfn++] = c;
        hfp[hfn] = 0;
    }
}

static void flush_space(void)
{
    unsigned char f = fmt;
    if (pend) { fmt = pendf; put(' '); fmt = f; pend = 0; }
    pstarted = 1;
}

static void __fastcall__ text(unsigned char c)
{
    if (!inpara || !dest) return;
    if (c <= ' ') {                     /* white space collapses */
        if (pstarted && !pend) { pend = 1; pendf = fmt; }
        return;
    }
    flush_space();
    put(c);
}

static void __fastcall__ texts(const char *s)
{
    while (*s) text(*s++);
}

/* a character from its Unicode code point */
static const char latin[] = "AAAAAAACEEEEIIIIDNOOOOOxOUUUUYTsaaaaaaaceeeeiiiidnooooo/ouuuuyty";
static const unsigned ucode[] = {
    0xA0, 0x2010, 0x2013, 0x2014, 0x2212, 0x2018, 0x2019, 0xB4, 0x201C, 0x201D, 0xAB, 0xBB,
    0x2022, 0x25CF, 0xB7, 0xB0, 0
};
static const char uchr[] = " ----\'\'\'\"\"\"\"**.o";
static void __fastcall__ uchar(unsigned u)
{
    unsigned char i;
    if (u < 0x80) { text(u); return; }
    if (u >= 0xC0 && u <= 0xFF) { text(latin[u - 0xC0]); return; }
    for (i = 0; ucode[i]; ++i)
        if (ucode[i] == u) { text(uchr[i]); return; }
    if (u == 0xAD || u == 0x200B || u == 0xFEFF) return;
    if (u == 0x2026) texts("...");
    else if (u == 0xA9) texts("(c)");
    else text('?');
}

/* ---------------------------------------------------------------------- */
/* elements                                                                */

static void p_start(void)
{
    unsigned char pa;
    memcpy(&R, &D, sizeof(R));
    R.pm = R.brk = 0;
    chain(sname ? sname : hash("Standard"));
    if (eid == E_H) R.b = 1;
    fmt = fmt_of();
    fsp = 0;
    inpara = 1;
    pend = pstarted = 0;
    if (dest != 1) return;
    if (firstp) {                       /* the first paragraph's indents set the ruler */
        firstp = 0;
        if (R.pm & PM_ML) lmargin = R.ml > 472 ? 472 : R.ml;
        if (R.pm & PM_MR) rmargin = R.mr > 512 - lmargin - 40 ? lmargin + 40 : 512 - R.mr;
    }
    if (R.brk && gs + on > 1) ob(0x0C);
    pa = (R.al << 2) + (R.sp << 4);
    lastpa = pa;
    if (pa) ob(0x83 + pa);
    if (listp) { listp = 0; texts("- "); }
}

static void para_end(void)
{
    inpara = 0;
    if (dest == 1) ob(CR);
    else if (dest >= 2) put(' ');
}

static void __fastcall__ start_tag(unsigned char empty)
{
    unsigned char e = eid;
    switch (e) {
    case E_STYLE:
    case E_DEFSTYLE:
        incs = 0;
        memset(&C, 0, sizeof(C));
        ntabs = 0;
        if (e == E_DEFSTYLE) { if (sfam == 1) { incs = 2; memcpy(&C, &D, sizeof(C)); } break; }
        if (!sfam || !sname || nst >= (stylexml ? NST - 50 : NST)) break;
        C.h = sname;
        C.par = spar;
        incs = 1;
        if (empty) { memcpy(&sty[nst++], &C, sizeof(C)); incs = 0; }
        break;
    case E_P:
    case E_H:
        if ((!inbody && !stylexml) || inpara) break;
        p_start();
        if (empty) para_end();
        break;
    case E_SPAN:
        if (!inpara || fsp >= 8) break;
        fstk[fsp++] = fmt;
        if (sname) {
            from_fmt(fmt);
            chain(sname);
            fmt = fmt_of();
        }
        if (empty) fmt = fstk[--fsp];
        break;
    case E_S:
        if (!inpara) break;
        flush_space();
        if (!scount) scount = 1;
        while (scount--) put(' ');
        break;
    case E_TAB:
        if (!inpara) break;
        flush_space();
        put(TAB);
        break;
    case E_LB:
        if (!inpara) break;
        para_end();
        if (dest == 1 && lastpa) ob(0x83 + lastpa);
        inpara = 1;
        pend = pstarted = 0;
        break;
    case E_LITEM: listp = 1; break;
    case E_BODY: inbody = 1; if (!stylexml) dest = 1; break;
    case E_MASTER: ++master; break;
    case E_HEADER:
    case E_FOOTER:
        if (!stylexml || master != 1 || empty) break;
        hfp = e == E_HEADER ? doc_head : doc_foot;
        hfn = 0;
        hfp[0] = 0;
        dest = e == E_HEADER ? 2 : 3;
        break;
    case E_PAGENUM:
        if (inpara && dest >= 2) { flush_space(); put('#'); }
        if (!empty) skipat = dep;
        break;
    case E_SKIP:
        if (!empty) skipat = dep;
        break;
    }
}

static void __fastcall__ end_tag(unsigned char e)
{
    unsigned char i;
    unsigned ml;
    switch (e) {
    case E_STYLE:
    case E_DEFSTYLE:
        if (incs && ntabs && !tabsdone && !stylexml) {   /* first tab stops in content.xml */
            ml = (C.pm & PM_ML) ? C.ml : 0;
            for (i = 0; i < ntabs; ++i) dtabs[i] = stab[i] + ml > 511 ? 511 : stab[i] + ml;
            tabsdone = 1;
        }
        if (incs == 1) memcpy(&sty[nst++], &C, sizeof(C));
        else if (incs == 2) memcpy(&D, &C, sizeof(C));
        incs = 0;
        break;
    case E_P:
    case E_H:
        if (inpara) para_end();
        break;
    case E_SPAN:
        if (fsp) fmt = fstk[--fsp];
        break;
    case E_HEADER:
    case E_FOOTER:
        if (dest >= 2) {
            while (hfn && hfp[hfn - 1] == ' ') hfp[--hfn] = 0;
            dest = 0;
        }
        break;
    case E_BODY: inbody = 0; break;
    }
}

/* ---------------------------------------------------------------------- */
/* the tokenizer                                                           */

/* after '&': the character's code point */
static unsigned entity(void)
{
    char e[10];
    unsigned char n = 0, c;
    unsigned u = 0;
    while ((c = nextc()) && c != ';' && n < 9) e[n++] = c;
    e[n] = 0;
    if (e[0] == '#') {
        if ((e[1] | 0x20) == 'x') {
            for (n = 2; e[n]; ++n)
                u = (u << 4) + (e[n] <= '9' ? e[n] - '0' : (e[n] | 0x20) - 'a' + 10);
        } else for (n = 1; e[n]; ++n) u = u * 10 + e[n] - '0';
        return u;
    }
    n = lookup(ents, e);
    return n == 0xFF ? '?' : "&<>\"'"[n];
}

/* reads to the end of a name; returns the character after it */
static unsigned char __fastcall__ name_into(char *b, unsigned char c)
{
    unsigned char n = 0;
    while (c > ' ' && c != '>' && c != '/' && c != '=') {
        if (n < 31) b[n++] = c;
        c = nextc();
    }
    b[n] = 0;
    return c;
}

static unsigned char __fastcall__ skipws(unsigned char c)
{
    while (c && c <= ' ') c = nextc();
    return c;
}

static void tag(void)
{
    unsigned char c = nextc(), n, q, endt = 0, empty = 0, skipping = skipat != 0;
    unsigned u;
    if (c == '?' || c == '!') {         /* declaration or comment */
        while ((c = nextc()) && c != '>') ;
        return;
    }
    if (c == '/') { endt = 1; c = nextc(); }
    c = name_into(tname, c);
    eid = skipping ? E_NONE : elem(tname);
    sname = spar = scount = 0;
    sfam = 0;
    /* attributes */
    for (;;) {
        c = skipws(c);
        if (!c) return;
        if (c == '>') break;
        if (c == '/') { empty = 1; c = nextc(); continue; }
        c = skipws(name_into(aname, c));
        if (c != '=') continue;
        q = skipws(nextc());
        n = 0;
        while ((c = nextc()) && c != q) {
            if (c == '&') { u = entity(); c = u < 0x80 ? u : '?'; }
            if (n < 47) aval[n++] = c;
        }
        aval[n] = 0;
        c = nextc();
        if (eid != E_NONE && (n = lookup(anames, aname)) != 0xFF) attr(n);
    }
    if (endt) {
        if (dep) --dep;
        if (skipat) { if (dep < skipat) skipat = 0; return; }
        end_tag(elem(tname));
        return;
    }
    if (!empty) ++dep;
    if (!skipping) start_tag(empty);
}

static void __fastcall__ parse(unsigned char k)
{
    unsigned char c, need;
    unsigned u;
    strcpy(tmppath, progdir);
    strcat(tmppath, "/RW2.XML1");
    tmppath[strlen(tmppath) - 1] = '1' + k;
    fd = open(tmppath, O_RDONLY);
    if (fd < 0) return;
    ip = il = 0;
    eof = dep = skipat = 0;
    stylexml = !k;
    inbody = inpara = dest = master = incs = 0;
    for (;;) {
        c = nextc();
        if (eof) break;
        if (c == '<') { tag(); continue; }
        if (skipat || !inpara) continue;
        if (c == '&') { uchar(entity()); continue; }
        if (c < 0x80) { text(c); continue; }
        /* UTF-8 */
        need = c >= 0xF0 ? 3 : c >= 0xE0 ? 2 : 1;
        u = c & (need == 1 ? 0x1F : 0x0F);
        if (need == 3) { nextc(); nextc(); nextc(); u = 0xFFFF; }
        else while (need--) u = (u << 6) + (nextc() & 0x3F);
        uchar(u);
    }
    close(fd);
    remove(tmppath);
}

/* RW2.XML1/2 -> the document; 1 if done */
unsigned char odt_read(void)
{
    text_clear();                       /* margins, header, footer back to defaults */
    memset(dtabs, 0, sizeof(dtabs));
    gs = 0; ge = CAP; tlen = 0;
    on = trunc_ = nst = tabsdone = listp = fsp = 0;
    firstp = 1;
    memset(&D, 0, sizeof(D));
    from_fmt(def_fmt);
    D.font = R.font;
    D.size = R.size;
    memcpy(&R, &D, sizeof(R));
    efmt = fmt_of();                    /* the document starts with a format mark */
    ob(0x80 | efmt);
    parse(0);
    parse(1);
    oflush();
    tlen = gs;
    if (tlen < 2) { ob(CR); oflush(); tlen = gs; }
    cur = 1;
    while (cur < tlen && (tch(cur) & 0x80)) ++cur;
    anc = cur;
    top = 0;
    topf = topp = 0;
    tf = fmt_at(cur);
    strcpy(docpath, pathbuf);
    doctype = 2;
    modified = 0;
    ruler_apply();
    redraw_all();
    if (trunc_) show_msg("The document was too long for\nRetroWrite II: the end was left out.");
    return 1;
}
