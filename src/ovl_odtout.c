/*
** RetroWrite II - overlay 8: the text of an .ODT file (content.xml)
**
** Written to RW2.XML2 in the RetroWrite II folder; overlay 11 then packs it
** with styles.xml into the .ODT (ZIP) file.
** Fonts map to Liberation Sans/Serif/Mono; 9/18/27 point screen sizes are
** written as 12/24/36 point, which keeps lines about as long as on paper.
** Paragraph styles P<hex> carry alignment, spacing, page breaks, the ruler's
** margins (as indents) and tab stops; text styles T<hex> carry the format byte.
*/
#pragma code-name (push, "OVL8")
#pragma rodata-name (push, "OVL8")
#pragma data-name (push, "OVL8")
#pragma bss-name (push, "OVL8BSS")

#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <apple2enh.h>
#include "rw.h"

static int fd;
static unsigned on;                     /* bytes waiting in iob */
static unsigned char pass, bad;

static void flush(void)
{
    if (!on) return;
    if (write(fd, iob, on) != (int)on) bad = 1;
    on = 0;
}

static void __fastcall__ oc(unsigned char c)
{
    if (!pass) return;
    iob[on++] = c;
    if (on == 512) flush();
}

static void __fastcall__ os(const char *s)
{
    if (!pass) return;
    while (*s) oc(*s++);
}

static void __fastcall__ ou(unsigned v)
{
    char b[6];
    unsigned char i = 5;
    b[5] = 0;
    do { b[--i] = '0' + v % 10; v /= 10; } while (v);
    os(b + i);
}

static void __fastcall__ ohex(unsigned char v)
{
    static const char hx[] = "0123456789ABCDEF";
    oc(hx[v >> 4]);
    oc(hx[v & 15]);
}

/* pixels at 80 per inch, as inches with three decimals */
static void __fastcall__ oin(unsigned px)
{
    unsigned t = px * 25 / 2;           /* thousandths */
    ou(t / 1000);
    oc('.');
    t %= 1000;
    oc('0' + t / 100);
    oc('0' + t / 10 % 10);
    oc('0' + t % 10);
    os("in\"");
}

/* attribute name="..." helper: writes ' name="' */
static void __fastcall__ oa_(const char *name)
{
    oc(' ');
    os(name);
    os("=\"");
}

static void __fastcall__ oxml(unsigned char c)
{
    if (c == '&') os("&amp;");
    else if (c == '<') os("&lt;");
    else if (c == '>') os("&gt;");
    else if (c >= 0x20 && c < 0x7F) oc(c);
    else oc('?');
}


#include "odtxml.h"

static unsigned char used_p[32], used_t[128];
static unsigned char fmt, pa, popen, pstart, brk, sopen, sfmt, sp, atstart, any;

static void close_span(void)
{
    if (sopen) os("</text:span>");
    sopen = 0;
}

static void open_p(void)
{
    unsigned char k = pa | (brk << 4);
    used_p[k] = 1;
    os("<text:p text:style-name=\"P");
    ohex(k);
    os("\">");
    popen = 1;
    pstart = brk = 0;
    sp = 0;
    atstart = 1;
    any = 1;
}

static void close_p(void)
{
    close_span();
    os("</text:p>");
    popen = 0;
}

static void __fastcall__ text_char(unsigned char c)
{
    if (!popen) open_p();
    if (!sopen || sfmt != fmt) {
        close_span();
        used_t[fmt] = 1;
        os("<text:span text:style-name=\"T");
        ohex(fmt);
        os("\">");
        sopen = 1;
        sfmt = fmt;
    }
    if (c == ' ') {
        os(atstart || sp ? "<text:s/>" : " ");
        sp = 1;
    } else if (c == TAB) {
        os("<text:tab/>");
        sp = 1;
        atstart = 0;
    } else {
        oxml(c);
        sp = atstart = 0;
    }
}

static void body(void)
{
    unsigned p, n, j;
    unsigned char c;
    fmt = 0;
    pa = popen = sopen = brk = any = 0;
    pstart = 1;
    for (p = 0; p < tlen; p += n) {
        n = tlen - p > 256 ? 256 : tlen - p;
        fetch(p, TB, n);
        for (j = 0; j < n; ++j) {
            c = TB[j];
            if (c & 0x80) {
                if ((c & 3) == 3) { if (pstart) pa = (c >> 2) & 0x0F; }
                else fmt = c & 0x7F;
            } else if (c == CR) {
                if (!popen) open_p();
                close_p();
                pstart = 1;
                pa = 0;
            } else if (c == 0x0C) {
                if (popen) close_p();
                brk = 1;
                pstart = 1;
            } else if (c == TAB || c >= 0x20) {
                text_char(c);
            }
        }
    }
    if (popen || !any) { if (!popen) open_p(); close_p(); }
}

static const char *const fontname[3] = { "Liberation Sans", "Liberation Serif", "Liberation Mono" };
static const char *const align_s[4] = { "start", "center", "end", "justify" };
static const char *const lh_s[4] = { "100%", "150%", "200%", "100%" };

static void auto_styles(void)
{
    unsigned char k, i, f, sz;
    for (k = 0; k < 32; ++k) {
        if (!used_p[k]) continue;
        os("<style:style style:name=\"P");
        ohex(k);
        os("\" style:family=\"paragraph\" style:parent-style-name=\"Standard\"><style:paragraph-properties");
        oa_("fo:margin-left"); oin(lmargin);
        oa_("fo:margin-right"); oin(512 - rmargin);
        oa_("fo:text-align"); os(align_s[k & 3]); oc('"');
        os(" style:justify-single-word=\"false\"");
        oa_("fo:line-height"); os(lh_s[(k >> 2) & 3]); oc('"');
        if (k & 0x10) os(" fo:break-before=\"page\"");
        oc('>');
        if (dtabs[0]) {
            os("<style:tab-stops>");
            for (i = 0; i < 8 && dtabs[i]; ++i) {
                if (dtabs[i] <= lmargin) continue;
                os("<style:tab-stop");
                oa_("style:position"); oin(dtabs[i] - lmargin);
                os("/>");
            }
            os("</style:tab-stops>");
        }
        os("</style:paragraph-properties></style:style>");
    }
    for (k = 0; k < 128; ++k) {
        if (!used_t[k]) continue;
        f = k & F_FONT;
        if (f > 2) f = 0;
        sz = (k & F_SIZE) >> 2;
        os("<style:style style:name=\"T");
        ohex(k);
        os("\" style:family=\"text\"><style:text-properties");
        oa_("style:font-name"); os(fontname[f]); oc('"');
        oa_("fo:font-size"); os(sz == 1 ? "24pt\"" : sz == 2 ? "36pt\"" : "12pt\"");
        if (sz == 3) os((k & F_BOLD) ? " style:text-position=\"sub 58%\"" : " style:text-position=\"super 58%\"");
        else if (k & F_BOLD) os(" fo:font-weight=\"bold\"");
        if (k & F_ITAL) os(" fo:font-style=\"italic\"");
        if (k & F_UND) os(" style:text-underline-style=\"solid\" style:text-underline-width=\"auto\" style:text-underline-color=\"font-color\"");
        os("/></style:style>");
    }
}

static void content(void)
{
    os(x_head);
    os("<office:document-content");
    os(x_ns);
    os(x_fonts);
    os("<office:automatic-styles>");
    auto_styles();
    os("</office:automatic-styles><office:body><office:text>");
    body();
    os("</office:text></office:body></office:document-content>");
}


/* the document -> RW2.XML2; 1 if done */
unsigned char odt_content(void)
{
    pshape = 2;
    ptr_hide(); ptr_show();
    /* pass 0: which paragraph and text styles are used */
    memset(used_p, 0, sizeof(used_p));
    memset(used_t, 0, sizeof(used_t));
    pass = 0;
    body();
    strcpy(tmppath, progdir);
    strcat(tmppath, "/RW2.XML2");
    _filetype = PRODOS_T_TXT;
    _auxtype = 0;
    fd = open(tmppath, O_WRONLY | O_CREAT | O_TRUNC);
    if (fd < 0) { pshape = 0; show_msg("Can't save the document:\nthe disk may be full or locked."); return 0; }
    bad = 0;
    on = 0;
    pass = 1;
    content();
    flush();
    if (close(fd) < 0) bad = 1;
    if (bad) {
        remove(tmppath);
        pshape = 0;
        show_msg("The document was not saved:\nthe disk may be full.");
        return 0;
    }
    return 1;
}
