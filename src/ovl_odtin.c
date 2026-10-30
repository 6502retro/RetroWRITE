/*
** RetroWrite II - overlay 9: unpack an .ODT file (a ZIP archive)
**
** styles.xml goes to RW2.XML1 and content.xml to RW2.XML2 in the
** RetroWrite II folder; overlay 10 then reads them into the document.
** Compressed parts are inflated (inflate.s) with the 32K history window in
** aux $4000-$BFFF, which is where the document and the font bitmaps live:
** the document is about to be replaced anyway, and the fonts are read back
** from RW2.FONTS afterwards.
*/
#pragma code-name (push, "OVL9")
#pragma rodata-name (push, "OVL9")
#pragma data-name (push, "OVL9")
#pragma bss-name (push, "OVL9BSS")

#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <apple2enh.h>
#include "rw.h"

/* inflate.s */
void inf_reset(void);
unsigned char inf_codes(void);
unsigned __fastcall__ inf_bits(unsigned char n);
unsigned char inf_byte(void);
void inf_align(void);
void __fastcall__ inf_put(unsigned char c);
unsigned __fastcall__ inf_dec(unsigned char table);
extern unsigned lcnt[16], lsym[288], dcnt[16], dsym[32];
extern unsigned char ob[512], tmpb[316];
extern unsigned obfill, cplen, cpdist, cpm;
extern unsigned char inf_err;

#define WIN 0x4000u

static int zfd, xfd;
static long at;
static unsigned long crem;              /* compressed bytes left to read */
static unsigned obase;                  /* bytes flushed so far (mod 64K) */
static unsigned dpages, dfrac, tpages;  /* progress, in 256-byte pages */
static unsigned char inflated, bar;

/* ---------------------------------------------------------------------- */
/* progress                                                                */

#define BX0 140
#define BW  280
#define BY  96

static void __fastcall__ moved(unsigned n)
{
    dfrac += n;
    dpages += dfrac >> 8;
    dfrac &= 0xFF;
}

static void progress(void)
{
    unsigned t = tpages, d = dpages;
    unsigned char w;
    while (t > 400) { t >>= 1; d >>= 1; }
    w = (unsigned char)(d * (BW / 2) / (t + 1));
    if (w > BW / 2) w = BW / 2;
    if (w == bar || (bar != 0xFF && w < BW / 2 && w < bar + 6)) return;
    bar = w;
    lb_clear(6);
    frame_rect(BX0, BX0 + BW + 4, 0, 5);
    if (w) lb_fill(BX0 + 2, BX0 + 2 + w * 2, 1, 4, LB_SET);
    present(BY, 6, BX0 / 7, (BX0 + BW + 4) / 7);
}

/* ---------------------------------------------------------------------- */
/* callbacks from inflate.s                                                */

/* the compressed bytes left, crem, as a 32-bit count without long math */
#define CREM ((unsigned char *)&crem)
unsigned inf_fill(void)
{
    unsigned n = 512;
    int r;
    if (!CREM[2] && !CREM[3] && CREM[1] < 2) n = *(unsigned *)CREM;
    if (!n) return 0;
    r = read(zfd, iob, n);
    if (r <= 0) return 0;
    if ((unsigned)r > *(unsigned *)CREM) { if (!CREM[2]--) --CREM[3]; }
    *(unsigned *)CREM -= r;
    return r;
}

void inf_flush(void)
{
    unsigned a, n = obfill, f;
    if (!n) return;
    if (write(xfd, ob, n) != (int)n) inf_err = 2;
    a = WIN + (obase & 0x7FFF);
    f = 0xC000u - a;
    if (f > n) f = n;
    aux_put(a, ob, f);
    if (n > f) aux_put(WIN, ob + f, n - f);
    obase += n;
    moved(n);
    obfill = 0;
    progress();
}

void inf_fetch(void)
{
    unsigned m = cpdist - obfill, a, f;
    if (m > cplen) m = cplen;
    a = WIN + ((obase - (cpdist - obfill)) & 0x7FFF);
    f = 0xC000u - a;
    if (f > m) f = m;
    aux_get(tmpb, a, f);
    if (m > f) aux_get(tmpb + f, WIN, m - f);
    cpm = m;
}

/* ---------------------------------------------------------------------- */
/* inflate                                                                 */

#define lens tmpb                       /* not needed at the same time */
#define offs ((unsigned *)(TB + 64))    /* 32 bytes after hdr */

static void __fastcall__ build(unsigned *cnt, unsigned *sym, const unsigned char *len, unsigned n)
{
    unsigned s;
    unsigned char l;
    memset(cnt, 0, 32);
    for (s = 0; s < n; ++s) ++cnt[len[s]];
    cnt[0] = 0;
    offs[1] = 0;
    for (l = 1; l < 15; ++l) offs[l + 1] = offs[l] + cnt[l];
    for (s = 0; s < n; ++s)
        if (len[s]) sym[offs[len[s]]++] = s;
}

static const unsigned char clorder[19] = { 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 };

static unsigned char dynamic(void)
{
    unsigned nlen, ndist, ncode, i, s, rep;
    unsigned char l;
    nlen = inf_bits(5) + 257;
    ndist = inf_bits(5) + 1;
    ncode = inf_bits(4) + 4;
    if (nlen > 286 || ndist > 30) return 1;
    memset(lens, 0, 19);
    for (i = 0; i < ncode; ++i) lens[clorder[i]] = inf_bits(3);
    build(dcnt, dsym, lens, 19);
    for (i = 0; i < nlen + ndist; ) {
        s = inf_dec(1);
        if (s > 18 || inf_err) return 1;
        if (s < 16) { lens[i++] = s; continue; }
        if (s == 16) {
            if (!i) return 1;
            l = lens[i - 1];
            rep = 3 + inf_bits(2);
        } else {
            l = 0;
            rep = s == 17 ? 3 + inf_bits(3) : 11 + inf_bits(7);
        }
        if (i + rep > nlen + ndist) return 1;
        while (rep--) lens[i++] = l;
    }
    if (!lens[256]) return 1;
    build(lcnt, lsym, lens, nlen);
    build(dcnt, dsym, lens + nlen, ndist);
    return inf_codes();
}

static unsigned char fixed(void)
{
    unsigned i;
    for (i = 0; i < 144; ++i) lens[i] = 8;
    for (; i < 256; ++i) lens[i] = 9;
    for (; i < 280; ++i) lens[i] = 7;
    for (; i < 288; ++i) lens[i] = 8;
    build(lcnt, lsym, lens, 288);
    for (i = 0; i < 30; ++i) lens[i] = 5;
    build(dcnt, dsym, lens, 30);
    return inf_codes();
}

static unsigned char stored(void)
{
    unsigned n, k;
    inf_align();
    n = inf_byte();
    n |= inf_byte() << 8;
    k = inf_byte();
    k |= inf_byte() << 8;
    if (n + k != 0xFFFFu) return 1;
    while (n-- && !inf_err) inf_put(inf_byte());
    return inf_err;
}

static unsigned char inflate(void)
{
    unsigned char last, t, r;
    inf_reset();
    obase = 0;
    do {
        last = inf_bits(1);
        t = inf_bits(2);
        if (t == 0) r = stored();
        else if (t == 1) r = fixed();
        else if (t == 2) r = dynamic();
        else r = 1;
        if (inf_err) return inf_err;
        if (r) return 1;
    } while (!last);
    inf_flush();
    return inf_err;
}

/* ---------------------------------------------------------------------- */
/* the ZIP directory                                                       */

#define hdr TB                          /* 46 bytes; nothing else uses TB meanwhile */
static unsigned long e_off[2], e_csize[2], e_usize[2];
static unsigned char e_meth[2], e_found[2];
static const char *const want[2] = { "styles.xml", "content.xml" };

static unsigned __fastcall__ g16(const unsigned char *p) { return p[0] | (p[1] << 8); }
#define g32(d, p) memcpy(&(d), (p), 4)

/* 0 ok, else a message */
static const char *directory(void)
{
    long sz = lseek(zfd, 0, SEEK_END);
    unsigned n, i, nent;
    unsigned long cd;
    unsigned char k;
    unsigned char *z = (unsigned char *)&sz;
    if (!z[2] && !z[3] && !z[1] && z[0] < 22) return "This is not an ODT document.";
    if (z[2] || z[3] || z[1] >= 2) {    /* the last 512 bytes */
        if (z[1] < 2 && !z[2]--) --z[3];
        z[1] -= 2;
    } else memset(z, 0, 4);
    lseek(zfd, sz, SEEK_SET);
    n = read(zfd, iob, 512);
    for (i = n - 22 + 1; i--; )
        if (iob[i] == 'P' && iob[i + 1] == 'K' && iob[i + 2] == 5 && iob[i + 3] == 6) break;
    if (i == 0xFFFFu) return "This is not an ODT document.";
    nent = g16(iob + i + 10);
    g32(cd, iob + i + 16);
    lseek(zfd, cd, SEEK_SET);
    e_found[0] = e_found[1] = 0;
    while (nent--) {
        if (read(zfd, hdr, 46) != 46 || hdr[0] != 'P' || hdr[2] != 1) break;
        n = g16(hdr + 28);
        if (n < 64) {
            read(zfd, iob, n);
            iob[n] = 0;
        } else {
            lseek(zfd, n, SEEK_CUR);
            iob[0] = 0;
        }
        for (k = 0; k < 2; ++k)
            if (!strcmp((char *)iob, want[k])) {
                if (hdr[8] & 1) return "This document is protected\nwith a password.";
                e_found[k] = 1;
                e_meth[k] = hdr[10];
                g32(e_csize[k], hdr + 20);
                g32(e_usize[k], hdr + 24);
                g32(e_off[k], hdr + 42);
            }
        lseek(zfd, g16(hdr + 30) + g16(hdr + 32), SEEK_CUR);
    }
    if (!e_found[1]) return "This is not an ODT document\n(it has no content.xml).";
    return 0;
}

static const char *__fastcall__ extract(unsigned char k)
{
    unsigned n;
    int r;
    char *s;
    strcpy(tmppath, progdir);
    strcat(tmppath, "/RW2.XML1");
    s = tmppath + strlen(tmppath) - 1;
    *s = '1' + k;
    remove(tmppath);
    if (!e_found[k]) return 0;
    if (e_meth[k] != 0 && e_meth[k] != 8) return "This ODT document is compressed\nin a way RetroWrite II can't read.";
    memcpy(&at, &e_off[k], 4);
    lseek(zfd, at, SEEK_SET);
    if (read(zfd, hdr, 30) != 30 || hdr[0] != 'P' || hdr[2] != 3) return "This ODT document is damaged.";
    lseek(zfd, g16(hdr + 26) + g16(hdr + 28), SEEK_CUR);
    _filetype = PRODOS_T_TXT;
    _auxtype = 0;
    xfd = open(tmppath, O_WRONLY | O_CREAT | O_TRUNC);
    if (xfd < 0) return "Can't write a work file in the\nRetroWrite II folder (disk full?).";
    memcpy(&crem, &e_csize[k], 4);
    if (e_meth[k] == 0) {
        while (crem) {
            n = inf_fill();
            if (!n || write(xfd, iob, n) != (int)n) { close(xfd); return "This ODT document is damaged."; }
            moved(n);
            progress();
        }
    } else {
        inflated = 1;
        r = inflate();
        if (r) { close(xfd); return r == 2 ? "The disk is full." : "This ODT document is damaged."; }
    }
    close(xfd);
    return 0;
}

/* put the font bitmaps back into aux $B700 */
static void reload_fonts(void)
{
    int fd, n;
    unsigned a = 0xB700;
    strcpy(tmppath, progdir);
    strcat(tmppath, "/RW2.FONTS");
    fd = open(tmppath, O_RDONLY);
    if (fd < 0) return;
    while ((n = read(fd, iob, 512)) > 0) {
        aux_put(a, iob, n);
        a += n;
    }
    close(fd);
}

/* pathbuf -> RW2.XML1 (styles) and RW2.XML2 (content); 1 if done */
unsigned char odt_unzip(void)
{
    const char *err;
    char *name;
    unsigned char k;
    ptr_hide();
    caret_hide();
    iob_limit = 2;                      /* the archive and a work file */
    upool_loaded = 0;
    zfd = open(pathbuf, O_RDONLY);
    if (zfd < 0) { iob_limit = 1; show_msg("Can't open the document."); return 0; }
    inflated = 0;
    dpages = dfrac = 0;
    bar = 0xFF;
    err = directory();
    if (!err) {
        name = strrchr(pathbuf, '/');
        draw_box(16, 63, 64, 112);
        lb_clear(12);
        ui_text(BX0, 9, F_BOLD, "Opening ");
        ui_text(gx, 9, F_BOLD, name ? name + 1 : pathbuf);
        present(74, 12, 17, 62);
        tpages = g16((unsigned char *)&e_usize[1] + 1) + (e_found[0] ? g16((unsigned char *)&e_usize[0] + 1) : 0);
        progress();
        for (k = 0; k < 2 && !err; ++k) err = extract(k);
    }
    close(zfd);
    iob_limit = 1;
    if (inflated) reload_fonts();
    if (err) {
        if (inflated) { text_clear(); docpath[0] = 0; doctype = 0; redraw_all(); }
        else { redraw_all(); refresh(); }
        show_msg(err);
        return 0;
    }
    return 1;
}
