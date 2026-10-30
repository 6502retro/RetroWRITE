/*
** RetroWrite II - overlay 11: pack an .ODT file (a ZIP archive with its
** parts stored, not compressed): mimetype, META-INF/manifest.xml,
** styles.xml (page, header and footer) and content.xml (from RW2.XML2,
** written by overlay 8).
*/
#pragma code-name (push, "OVLB")
#pragma rodata-name (push, "OVLB")
#pragma data-name (push, "OVLB")
#pragma bss-name (push, "OVLBBSS")

#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <apple2enh.h>
#include "rw.h"

void crc_init(void);
void __fastcall__ crc_update(const void *buf, unsigned n);
extern unsigned long crcv;

static int fd;
static unsigned on;                     /* bytes waiting in iob */
static unsigned long fpos, usize;     /* (32-bit sums kept by add32) */
static unsigned char pass, bad;

#define NENT 4
static unsigned long e_off[NENT], e_crc[NENT], e_size[NENT];
static const char *const e_name[NENT] = {
    "mimetype", "META-INF/manifest.xml", "styles.xml", "content.xml"
};
static unsigned dosdate, dostime;

/* v += n without the long-arithmetic library */
static void __fastcall__ add32(unsigned long *v, unsigned n)
{
    unsigned char *p = (unsigned char *)v;
    unsigned s = p[0] + (n & 0xFF);
    p[0] = s;
    s = p[1] + (n >> 8) + (s >> 8);
    p[1] = s;
    if (s >> 8) { if (!++p[2]) ++p[3]; }
}

static void flush(void)
{
    if (!on) return;
    crc_update(iob, on);
    if (write(fd, iob, on) != (int)on) bad = 1;
    add32(&fpos, on);
    add32(&usize, on);
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

static void __fastcall__ oxml(unsigned char c)
{
    if (c == '&') os("&amp;");
    else if (c == '<') os("&lt;");
    else if (c == '>') os("&gt;");
    else if (c >= 0x20 && c < 0x7F) oc(c);
    else oc('?');
}

static void __fastcall__ wr(const void *p, unsigned n)
{
    if (write(fd, p, n) != (int)n) bad = 1;
    add32(&fpos, n);
}

static void __fastcall__ le16(unsigned char *p, unsigned v) { p[0] = v; p[1] = v >> 8; }
#define le32(p, v) memcpy((p), &(v), 4)

/* local header (30 bytes + name); sizes and CRC are filled in afterwards */
static void __fastcall__ begin(unsigned char k)
{
    unsigned char h[30];
    unsigned n = strlen(e_name[k]);
    memset(h, 0, 30);
    h[0] = 'P'; h[1] = 'K'; h[2] = 3; h[3] = 4;
    h[4] = 20;
    le16(h + 10, dostime);
    le16(h + 12, dosdate);
    le16(h + 26, n);
    memcpy(&e_off[k], &fpos, 4);
    wr(h, 30);
    wr(e_name[k], n);
    memset(&crcv, 0xFF, 4);
    memset(&usize, 0, 4);
    pass = 1;
}

static void __fastcall__ end(unsigned char k)
{
    unsigned char h[12], i;
    unsigned long at;
    flush();
    for (i = 0; i < 4; ++i) ((unsigned char *)&e_crc[k])[i] = ((unsigned char *)&crcv)[i] ^ 0xFF;
    memcpy(&e_size[k], &usize, 4);
    le32(h, e_crc[k]);
    le32(h + 4, usize);
    le32(h + 8, usize);
    memcpy(&at, &e_off[k], 4);
    add32(&at, 14);
    lseek(fd, at, SEEK_SET);
    if (write(fd, h, 12) != 12) bad = 1;
    lseek(fd, 0, SEEK_END);
}

static void central(void)
{
    unsigned char h[46], k;
    unsigned long cd;
    unsigned n;
    memcpy(&cd, &fpos, 4);
    for (k = 0; k < NENT; ++k) {
        n = strlen(e_name[k]);
        memset(h, 0, 46);
        h[0] = 'P'; h[1] = 'K'; h[2] = 1; h[3] = 2;
        h[4] = 20; h[6] = 20;
        le16(h + 12, dostime);
        le16(h + 14, dosdate);
        le32(h + 16, e_crc[k]);
        le32(h + 20, e_size[k]);
        le32(h + 24, e_size[k]);
        le16(h + 28, n);
        le32(h + 42, e_off[k]);
        wr(h, 46);
        wr(e_name[k], n);
    }
    memset(h, 0, 22);
    h[0] = 'P'; h[1] = 'K'; h[2] = 5; h[3] = 6;
    h[8] = h[10] = NENT;
    le16(h + 12, (unsigned)fpos - (unsigned)cd);
    le32(h + 16, cd);
    wr(h, 22);
}

#include "odtxml.h"

static void manifest(void)
{
    os(x_head);
    os("<manifest:manifest xmlns:manifest=\"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0\" manifest:version=\"1.2\">"
       "<manifest:file-entry manifest:full-path=\"/\" manifest:version=\"1.2\" manifest:media-type=\"application/vnd.oasis.opendocument.text\"/>"
       "<manifest:file-entry manifest:full-path=\"content.xml\" manifest:media-type=\"text/xml\"/>"
       "<manifest:file-entry manifest:full-path=\"styles.xml\" manifest:media-type=\"text/xml\"/>"
       "</manifest:manifest>");
}

/* header or footer text: # is the page number */
static void __fastcall__ hf(const char *tag, const char *s)
{
    os("<style:");
    os(tag);
    os("><text:p text:style-name=\"RW_HF\">");
    for (; *s; ++s)
        if (*s == '#') os("<text:page-number text:select-page=\"current\">1</text:page-number>");
        else oxml(*s);
    os("</text:p></style:");
    os(tag);
    oc('>');
}

static void styles(void)
{
    os(x_head);
    os("<office:document-styles");
    os(x_ns);
    os(x_fonts);
    os("<office:styles><style:default-style style:family=\"paragraph\">"
       "<style:paragraph-properties style:tab-stop-distance=\"0.5in\"/>"
       "<style:text-properties style:font-name=\"Liberation Sans\" fo:font-size=\"12pt\"/></style:default-style>"
       "<style:style style:name=\"Standard\" style:family=\"paragraph\" style:class=\"text\"/>"
       "<style:style style:name=\"RW_HF\" style:family=\"paragraph\" style:parent-style-name=\"Standard\">"
       "<style:paragraph-properties fo:text-align=\"center\"/></style:style></office:styles>"
       "<office:automatic-styles><style:page-layout style:name=\"pm1\"><style:page-layout-properties"
       " fo:page-width=\"8.5in\" fo:page-height=\"11in\" style:print-orientation=\"portrait\""
       " fo:margin-top=\"0.75in\" fo:margin-bottom=\"0.75in\" fo:margin-left=\"1in\" fo:margin-right=\"1.1in\"/>"
       "<style:header-style><style:header-footer-properties fo:min-height=\"0in\" fo:margin-bottom=\"0.25in\"/></style:header-style>"
       "<style:footer-style><style:header-footer-properties fo:min-height=\"0in\" fo:margin-top=\"0.25in\"/></style:footer-style>"
       "</style:page-layout></office:automatic-styles>"
       "<office:master-styles><style:master-page style:name=\"Standard\" style:page-layout-name=\"pm1\">");
    if (doc_head[0]) hf("header", doc_head);
    if (doc_foot[0]) hf("footer", doc_foot);
    else if (prt_pnum) hf("footer", "- # -");
    os("</style:master-page></office:master-styles></office:document-styles>");
}

/* ---------------------------------------------------------------------- */

static void get_date(void)
{
    unsigned d = PEEK(0xBF90) | (PEEK(0xBF91) << 8);
    unsigned y = d >> 9, m = (d >> 5) & 15, dd = d & 31;
    if (!d || !m || m > 12 || !dd) { dosdate = (46u << 9) | (1 << 5) | 1; dostime = 0; return; }
    y += y < 40 ? 2000 : 1900;
    dosdate = ((y - 1980) << 9) | (m << 5) | dd;
    dostime = ((PEEK(0xBF93) & 31) << 11) | ((PEEK(0xBF92) & 63) << 5);
}

/* RW2.XML2 + styles -> pathbuf as ODT; 1 if done */
unsigned char odt_save(void)
{
    char *slash;
    int xfd, n;
    crc_init();
    get_date();
    strcpy(tmppath, progdir);
    strcat(tmppath, "/RW2.XML2");
    iob_limit = 2;                      /* RW2.XML2 and the new file */
    upool_loaded = 0;
    xfd = open(tmppath, O_RDONLY);
    if (xfd < 0) { iob_limit = 1; pshape = 0; return 0; }
    strcpy(tmppath, pathbuf);
    slash = strrchr(tmppath, '/');
    strcpy(slash ? slash + 1 : tmppath, "RW2.SAVE.TMP");
    remove(tmppath);
    _filetype = PRODOS_T_BIN;
    _auxtype = 0;
    fd = open(tmppath, O_WRONLY | O_CREAT | O_TRUNC);
    if (fd < 0) { close(xfd); iob_limit = 1; pshape = 0; show_msg("Can't save the document:\nthe disk may be full or locked."); return 0; }
    bad = 0;
    memset(&fpos, 0, 4);
    on = 0;
    begin(0); os("application/vnd.oasis.opendocument.text"); end(0);
    begin(1); manifest(); end(1);
    begin(2); styles(); end(2);
    begin(3);
    while ((n = read(xfd, iob, 512)) > 0) { on = n; flush(); }
    end(3);
    close(xfd);
    pass = 0;
    central();
    if (close(fd) < 0) bad = 1;
    iob_limit = 1;
    pshape = 0;
    if (bad) {
        remove(tmppath);
        show_msg("The document was not saved:\nthe disk may be full.");
        return 0;
    }
    remove(pathbuf);
    if (rename(tmppath, pathbuf) < 0) { show_msg("Can't rename the saved file."); return 0; }
    strcpy(tmppath, progdir);
    strcat(tmppath, "/RW2.XML2");
    remove(tmppath);
    strcpy(docpath, pathbuf);
    doctype = 2;
    modified = 0;
    draw_menubar();
    return 1;
}
