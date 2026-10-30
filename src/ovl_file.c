/*
** RetroWrite II - overlay 1: loading and saving, native and text formats
*/
#pragma code-name (push, "OVL1")
#pragma rodata-name (push, "OVL1")
#pragma data-name (push, "OVL1")
#pragma bss-name (push, "OVL1BSS")

#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <device.h>
#include <errno.h>
#include <apple2enh.h>
#include "rw.h"

#define RW_FILETYPE 0x50        /* word processor document */
#define RW_AUXTYPE  0x5257      /* 'RW' */


static const char magic[4] = { 'R', 'W', '2', 'D' };

/* ---------------------------------------------------------------------- */

static void __fastcall__ oserr(const char *what)
{
    static char m[80];
    unsigned char e = _oserror;
    const char *s;
    switch (e) {
    case 0x27: s = "I/O error"; break;
    case 0x2B: s = "the disk is write-protected"; break;
    case 0x40: s = "invalid pathname"; break;
    case 0x44: s = "folder not found"; break;
    case 0x45: s = "volume not found"; break;
    case 0x46: s = "file not found"; break;
    case 0x48: s = "the disk is full"; break;
    case 0x49: s = "the folder is full"; break;
    case 0x4E: s = "the file is locked"; break;
    default: s = "ProDOS error"; break;
    }
    strcpy(m, what);
    strcat(m, ":\n");
    strcat(m, s);
    show_msg(m);
}

unsigned char __fastcall__ file_exists(const char *path)
{
    int fd = open(path, O_RDONLY);
    if (fd < 0) return 0;
    close(fd);
    return 1;
}

/* type: 4 = text, otherwise RetroWrite document */
unsigned char __fastcall__ load_file(const char *path, unsigned char type)
{
    int fd, n;
    long sz;
    unsigned m, i;
    unsigned char hdr[5], c;
    fd = open(path, O_RDONLY);
    if (fd < 0) { oserr("Can't open the document"); return 0; }
    sz = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    if (sz > (long)CAP - 2) {
        close(fd);
        show_msg("This document is too large for\nRetroWrite II (30,460 characters).");
        return 0;
    }
    if (type != 4) {
        if (read(fd, hdr, 5) != 5 || memcmp(hdr, magic, 4) || hdr[4] > 3) {
            close(fd);
            show_msg("This is not a RetroWrite II document.");
            return 0;
        }
    }
    text_clear();                       /* leaves the base format marker */
    if (type != 4 && hdr[4] >= 2) {     /* version 2: the ruler follows */
        read(fd, iob, 20);
        memcpy(&lmargin, iob, 2);
        memcpy(&rmargin, iob + 2, 2);
        memcpy(dtabs, iob + 4, 16);
        if (rmargin > 512 || lmargin + 40 > rmargin) { lmargin = 0; rmargin = 512; }
    }
    if (type != 4 && hdr[4] == 3) {     /* version 3: header and footer too */
        read(fd, doc_head, 40);
        read(fd, doc_foot, 40);
        doc_head[39] = doc_foot[39] = 0;
    }
    gs = 0; ge = CAP; tlen = 0;         /* start empty */
    fprev = 0;
    if (type != 4) {
        while ((n = read(fd, iob, 512)) > 0) {
            m = 0;
            for (i = 0; i < (unsigned)n; ++i) {
                c = iob[i];
                if (c >= 0x80 || c == CR || c == TAB || (c >= 0x20 && c < 0x7F)) iob[m++] = c;
            }
            aux_put(TBASE + gs, iob, m);
            gs += m;
        }
    } else {
        iob[0] = 0x80;
        aux_put(TBASE, iob, 1);
        gs = 1;
        while ((n = read(fd, iob, 512)) > 0) {
            m = filt(n);
            aux_put(TBASE + gs, iob, m);
            gs += m;
        }
    }
    close(fd);
    tlen = gs;
    if (!tlen || !(tch(0) & 0x80)) {    /* make sure a format comes first */
        static const unsigned char base = 0x80;
        raw_insert(0, &base, 1);
    }
    cur = anc = 1;
    while (cur < tlen && (tch(cur) & 0x80)) ++cur;
    anc = cur;
    top = 0;
    topf = topp = 0;
    tf = fmt_at(cur);
    strcpy(docpath, path);
    doctype = (type == 4);
    modified = 0;
    redraw_all();
    return 1;
}

static unsigned char has_formatting(void)
{
    unsigned p, n;
    unsigned char i;
    for (p = 1; p < tlen; p += n) {
        n = tlen - p > 256 ? 256 : tlen - p;
        fetch(p, iob, n);
        for (i = 0; i < (unsigned char)n; ++i)
            if ((iob[i] & 0x80) && iob[i] != 0x80) return 1;
        if (n < 256) break;
    }
    return tlen && (tch(0) & 0x7F);
}

static unsigned char __fastcall__ save_file(const char *path, unsigned char astext)
{
    int fd;
    unsigned p, n, m, i;
    char *slash;
    strcpy(tmppath, path);
    slash = strrchr(tmppath, '/');
    slash = slash ? slash + 1 : tmppath;
    strcpy(slash, "RW2.SAVE.TMP");
    remove(tmppath);
    if (astext) { _filetype = PRODOS_T_TXT; _auxtype = 0; }
    else { _filetype = RW_FILETYPE; _auxtype = RW_AUXTYPE; }
    pshape = 2;
    ptr_hide(); ptr_show();
    fd = open(tmppath, O_WRONLY | O_CREAT | O_TRUNC);
    if (fd < 0) { oserr("Can't save the document"); return 0; }
    if (!astext) {
        iob[0] = 'R'; iob[1] = 'W'; iob[2] = '2'; iob[3] = 'D'; iob[4] = 3;
        memcpy(iob + 5, &lmargin, 2);
        memcpy(iob + 7, &rmargin, 2);
        memcpy(iob + 9, dtabs, 16);
        memcpy(iob + 25, doc_head, 40);
        memcpy(iob + 65, doc_foot, 40);
        if (write(fd, iob, 105) != 105) goto fail;
    }
    for (p = 0; p < tlen; p += n) {
        n = tlen - p > 512 ? 512 : tlen - p;
        fetch(p, iob, n);
        m = n;
        if (astext) {
            m = 0;
            for (i = 0; i < n; ++i)
                if (!(iob[i] & 0x80)) iob[m++] = iob[i];
        }
        if (m && write(fd, iob, m) != (int)m) goto fail;
    }
    if (close(fd) < 0) { fd = -1; goto fail; }
    if (file_exists(path) && remove(path) < 0) { oserr("Can't replace the old file"); remove(tmppath); return 0; }
    if (rename(tmppath, path) < 0) { oserr("Can't rename the saved file"); return 0; }
    strcpy(docpath, path);
    doctype = astext;
    modified = 0;
    return 1;
fail:
    if (fd >= 0) close(fd);
    remove(tmppath);
    oserr("The document was not saved");
    return 0;
}

/* ---------------------------------------------------------------------- */
/* Operations (the resident part strings them together with the dialog)   */

static unsigned char __fastcall__ do_save(unsigned char astext)
{
    unsigned char r;
    strcpy(tmppath, pathbuf);
    strcpy(pathbuf, tmppath);
    r = save_file(pathbuf, astext);
    pshape = 0;
    if (r) draw_menubar();
    return r;
}

/* open a file from the RetroWrite II folder (type as for load_file);
   the tutorial opens as an untitled copy */
static void __fastcall__ load_special(const char *name, unsigned char type)
{
    strcpy(pathbuf, progdir);
    strcat(pathbuf, name);
    if (file_exists(pathbuf)) {
        strcpy(tmppath, pathbuf);
        if (load_file(tmppath, type) && !type) { docpath[0] = 0; draw_menubar(); }
    } else if (type) {
        text_clear();
        strcpy(docpath, pathbuf);
        doctype = 1;
        redraw_all();
    } else show_msg("The TUTORIAL document is missing\nfrom the RetroWrite II folder.");
}

/* ---------------------------------------------------------------------- */
/* Windows: a hidden document is written to RW2.WINn with its state       */

static struct {
    unsigned tlen, cur, anc, top, lmargin, rmargin, dtabs[8];
    unsigned char topf, topp, tf, modified, doctype;
    char docpath[65], head[40], foot[40];
} st;

static void __fastcall__ win_file(unsigned char k)
{
    strcpy(tmppath, progdir);
    strcat(tmppath, "/RW2.WIN0");
    tmppath[strlen(tmppath) - 1] = '0' + k;
}

static unsigned char swap_out(void)
{
    int fd;
    unsigned p, n;
    unsigned char k = cur_doc;
    const char *b = docpath[0] ? strrchr(docpath, '/') + 1 : "Untitled";
    strncpy(wname[k], b, 15);
    wname[k][15] = 0;
    if (modified) wmod |= 1 << k; else wmod &= ~(1 << k);
    st.tlen = tlen; st.cur = cur; st.anc = anc; st.top = top;
    st.lmargin = lmargin; st.rmargin = rmargin;
    memcpy(st.dtabs, dtabs, 16);
    st.topf = topf; st.topp = topp; st.tf = tf; st.modified = modified; st.doctype = doctype;
    strcpy(st.docpath, docpath);
    strcpy(st.head, doc_head);
    strcpy(st.foot, doc_foot);
    win_file(k);
    _filetype = PRODOS_T_BIN;
    _auxtype = 0;
    fd = open(tmppath, O_WRONLY | O_CREAT | O_TRUNC);
    if (fd < 0) { oserr("Can't put the document away"); return 0; }
    if (write(fd, &st, sizeof(st)) != sizeof(st)) goto bad;
    for (p = 0; p < tlen; p += n) {
        n = tlen - p > 512 ? 512 : tlen - p;
        fetch(p, iob, n);
        if (write(fd, iob, n) != (int)n) goto bad;
    }
    close(fd);
    return 1;
bad:
    close(fd);
    oserr("Can't put the document away");
    return 0;
}

static unsigned char swap_in(void)
{
    int fd, n;
    unsigned char k = fo_arg;
    cur_doc = k;
    text_clear();
    docpath[0] = 0;
    doctype = 0;
    if (wlive & (1 << k)) {
        win_file(k);
        fd = open(tmppath, O_RDONLY);
        if (fd >= 0 && read(fd, &st, sizeof(st)) == sizeof(st)) {
            gs = 0;
            while ((n = read(fd, iob, 512)) > 0) {
                aux_put(TBASE + gs, iob, n);
                gs += n;
            }
            tlen = gs;
            cur = st.cur; anc = st.anc; top = st.top;
            lmargin = st.lmargin; rmargin = st.rmargin;
            memcpy(dtabs, st.dtabs, 16);
            topf = st.topf; topp = st.topp; tf = st.tf; modified = st.modified; doctype = st.doctype;
            strcpy(docpath, st.docpath);
            strcpy(doc_head, st.head);
            strcpy(doc_foot, st.foot);
        }
        if (fd >= 0) close(fd);
        if (tlen != st.tlen) {          /* damaged: start empty */
            text_clear();
            show_msg("A hidden document could not be read back.");
        }
    }
    wlive |= 1 << k;
    redraw_all();
    return 1;
}

/* close the current window; a later window moves into its place so that
   the Windows menu has no gaps */
static unsigned char close_win(void)
{
    unsigned char k = cur_doc, m;
    wlive &= ~(1 << k);
    wmod &= ~(1 << k);
    for (m = 3; m > k && !(wlive & (1 << m)); --m) ;
    if (m > k) {
        win_file(k);
        strcpy(pathbuf, tmppath);
        remove(pathbuf);
        win_file(m);
        rename(tmppath, pathbuf);
        strcpy(wname[k], wname[m]);
        if (wmod & (1 << m)) wmod |= 1 << k;
        wmod &= ~(1 << m);
        wlive &= ~(1 << m);
        wlive |= 1 << k;
        fo_arg = k;
    } else {
        for (m = 0; m < k && !(wlive & (1 << m)); ++m) ;
        if (m == k) {                   /* it was the only window */
            wlive = 1;
            cur_doc = 0;
            return file_op(FO_NEW);
        }
        fo_arg = m;
    }
    return swap_in();
}

unsigned char __fastcall__ file_op(unsigned char op)
{
    unsigned char r;
    switch (op) {
    case FO_NEW:
        text_clear();
        docpath[0] = 0;
        doctype = 0;
        redraw_all();
        return 1;
    case FO_LOAD:
        strcpy(tmppath, pathbuf);
        return load_file(tmppath, ftype == 0x04 ? 4 : 0);
    case FO_SAVETO: return do_save(0);
    case FO_SAVETEXT: return do_save(1);
    case FO_TEXTOK:
        return !has_formatting() ||
            !alert("A text file can't keep fonts, sizes\nor styles. Save it as text anyway?", "Save", "Cancel", 0);
    case FO_SAVE:
        if (!docpath[0]) return 2;
        if (doctype == 1 && has_formatting()) {
            r = alert("This document came from a text file,\nwhich can't keep fonts and styles.\nSave as a RetroWrite document?",
                      "RetroWrite", "Text", "Cancel");
            if (r == 2) return 0;
            if (r == 0) return 2;
        }
        strcpy(pathbuf, docpath);
        if (doctype == 2) return 3;     /* ODT: overlay 8 */
        return do_save(doctype);
    case FO_SWAPOUT: return swap_out();
    case FO_SWAPIN: return swap_in();
    case FO_CLOSEWIN: return close_win();
    case FO_UDICT: load_special("/USER.DICT", 4); return 1;
    case FO_GLOSS: load_special("/GLOSSARY", 4); return 1;
    case FO_TUTOR: load_special("/TUTORIAL", 0); return 1;
    }
    return 0;
}
