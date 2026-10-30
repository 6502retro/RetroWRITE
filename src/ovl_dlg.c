/*
** RetroWrite II - overlay 7: the Open/Save file dialog
*/
#pragma code-name (push, "OVL7")
#pragma rodata-name (push, "OVL7")
#pragma data-name (push, "OVL7")
#pragma bss-name (push, "OVL7BSS")

#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <device.h>
#include <apple2enh.h>
#include "rw.h"

#define RW_FILETYPE 0x50        /* word processor document */
#define RW_AUXTYPE  0x5257      /* 'RW' */

static unsigned char __fastcall__ exists(const char *path)
{
    int fd = open(path, O_RDONLY);
    if (fd < 0) return 0;
    close(fd);
    return 1;
}

/* ---------------------------------------------------------------------- */
/* File dialog                                                             */

struct ent {
    char name[16];
    unsigned char type;         /* 0x0F folder, 0x04 text, 0x50 document, 0xFF volume */
    unsigned size;
};
/* entries live in text page 1 ($0400-$07FF, unused in double hi-res):
   6 per 128-byte block, keeping clear of the screen holes at $x78-$x7F */
#define ENT(i) ((struct ent *)(0x400 + ((i) / 6) * 0x80 + ((i) % 6) * 19))
static unsigned char nents;

/* dialog geometry (byte columns 10..69, x 70..489) */
#define FC0 10
#define FC1 69
#define FX0 (FC0 * 7)
#define FX1 (FC1 * 7 + 7)
#define FY0 26
#define FY1 178
#define LX0 (FX0 + 14)
#define LX1 (FX0 + 266)
#define LY0 58
#define LROWS 9
#define LY1 (LY0 + LROWS * 11 + 2)
#define BTX0 (FX0 + 290)
#define BTX1 (FX0 + 396)

static void sort_ents(void)
{
    unsigned char i, j;
    struct ent t;
    for (i = 1; i < nents; ++i) {
        t = (*ENT(i));
        j = i;
        while (j && strcmp((*ENT(j - 1)).name, t.name) > 0) {
            (*ENT(j)) = (*ENT(j - 1));
            --j;
        }
        (*ENT(j)) = t;
    }
}

/* read a ProDOS directory file directly (no heap needed) */
static void read_dir(void)
{
    int fd;
    unsigned char *e, k, n, first, l, t;
    fd = open(browsedir, O_RDONLY);
    if (fd < 0) { browsedir[0] = 0; return; }
    first = 1;
    while (nents < MAXENT && read(fd, iob, 512) == 512) {
        e = iob + 4;
        for (k = 0; k < 13; ++k, e += 0x27) {
            if (first && k == 0) continue;          /* directory header */
            n = e[0];
            l = n & 0x0F;
            if (!(n & 0xF0) || !l) continue;        /* deleted */
            t = e[0x10];
            if (t == 0x0F || t == 0x04 ||
                (t == RW_FILETYPE && e[0x1F] == (RW_AUXTYPE & 0xFF) && e[0x20] == (RW_AUXTYPE >> 8))) {
                memcpy((*ENT(nents)).name, e + 1, l);
                (*ENT(nents)).name[l] = 0;
                (*ENT(nents)).type = t;
                (*ENT(nents)).size = e[0x17] ? 0xFFFFu : e[0x15] | (e[0x16] << 8);
                if (++nents >= MAXENT) break;
            }
        }
        first = 0;
    }
    close(fd);
}

static void load_dir(void)
{
    unsigned char dev;
    nents = 0;
    if (browsedir[0]) read_dir();
    if (!browsedir[0]) {
        dev = getfirstdevice();
        while (dev != INVALID_DEVICE && nents < MAXENT) {
            if (getdevicedir(dev, tmppath, 80)) {
                strncpy((*ENT(nents)).name, tmppath + 1, 15);
                (*ENT(nents)).name[15] = 0;
                (*ENT(nents)).type = 0xFF;
                ++nents;
            }
            dev = getnextdevice(dev);
        }
    }
    sort_ents();
}

static unsigned char sel, scr, fmode, pmode;     /* fmode: 0 open, 1 save, 2 save as text */
static char fname[16];

static void draw_list(void)
{
    unsigned char i, n, y = LY0;
    struct ent *e;
    ptr_hide();
    /* frame top */
    lb_clear(1);
    lb_fill(LX0, LX1, 0, 0, LB_SET);
    present(y - 1, 1, LX0 / 7, (LX1 - 1) / 7);
    for (i = 0; i < LROWS; ++i, y += 11) {
        lb_clear(11);
        lb_fill(LX0, LX0 + 2, 0, 10, LB_SET);
        lb_fill(LX1 - 2, LX1, 0, 10, LB_SET);
        lb_fill(LX1 - 18, LX1 - 16, 0, 10, LB_SET);     /* list scroll bar edge */
        if (i == 0) ui_text(LX1 - 13, 8, 0, "^");
        else if (i == LROWS - 1) ui_text(LX1 - 13, 8, 0, "v");
        else lb_fill(LX1 - 16, LX1 - 2, 0, 10, LB_GRAY);
        n = scr + i;
        if (n < nents) {
            e = ENT(n);
            if (e->type == 0x04) ui_text(LX0 + 6, 8, 0, "\x83");
            else if (e->type == RW_FILETYPE) ui_text(LX0 + 6, 8, 0, "\x83");
            else ui_text(LX0 + 4, 8, 0, "\x82");
            ui_text(LX0 + 22, 8, e->type == RW_FILETYPE ? F_BOLD : 0, e->name);
            if (n == sel) lb_fill(LX0 + 2, LX1 - 18, 0, 10, LB_XOR);
        } else if (n == 0) {
            ui_text(LX0 + 22, 8, F_ITAL, browsedir[0] ? "(no documents)" : "(no disks)");
        }
        present(y, 11, LX0 / 7, (LX1 - 1) / 7);
    }
    lb_clear(1);
    lb_fill(LX0, LX1, 0, 0, LB_SET);
    present(y, 1, LX0 / 7, (LX1 - 1) / 7);
}

static void draw_folder(void)
{
    lb_clear(11);
    ui_text(FX0 + 14, 8, 0, "\x82 ");
    ui_text(gx, 8, F_BOLD, browsedir[0] ? browsedir : "Disks");
    present(42, 11, FC0 + 1, FC1 - 1);
}

static void draw_name(void)
{
    if (!fmode) return;
    lb_clear(13);
    ui_text(FX0 + 14, 9, 0, "Name:");
    frame_rect(FX0 + 60, LX1, 0, 12);
    ui_text(FX0 + 66, 9, 0, fname);
    lb_fill(gx + 1, gx + 2, 2, 10, LB_SET);
    present(LY1 + 4, 13, FC0 + 1, LX1 / 7);
}

static const char *const fbtn_open[] = { "Open", "Up", "Disks", "Cancel" };
static const char *const fbtn_save[] = { "Save", "Up", "Disks", "Cancel" };

static void draw_dialog(void)
{
    unsigned char i;
    const char *const *lab = fmode ? fbtn_save : fbtn_open;
    lb_clear(11);
    ui_text(FX0 + 14, 8, F_BOLD, pmode ? "Print Which Document? (Cancel when done)" : fmode == 0 ? "Open a Document" : fmode == 3 ? "Print Which Document? (Cancel when done)" :
            fmode == 1 ? "Save Document As" : "Save As Text (no fonts or styles)");
    present(30, 11, FC0 + 1, FC1 - 1);
    draw_folder();
    draw_list();
    draw_name();
    nbtn = 4;
    for (i = 0; i < 4; ++i) {
        blabel[i] = lab[i];
        bx0[i] = BTX0;
        bx1[i] = BTX1;
        by[i] = LY0 + i * 24 + (i == 3 ? 12 : 0);
        draw_button(BTX0, BTX1, by[i], lab[i], i == 0);
    }
}

static void enter(struct ent *e)
{
    if (e->type == 0xFF) {
        browsedir[0] = '/';
        strcpy(browsedir + 1, e->name);
    } else {
        if (strlen(browsedir) + strlen(e->name) + 1 > 63) { beep(); return; }
        strcat(browsedir, "/");
        strcat(browsedir, e->name);
    }
    load_dir();
    sel = scr = 0;
    draw_folder();
    draw_list();
}

static void go_up(void)
{
    char *s = strrchr(browsedir, '/');
    if (!browsedir[0]) return;
    if (s == browsedir || !s) browsedir[0] = 0;
    else *s = 0;
    load_dir();
    sel = scr = 0;
    draw_folder();
    draw_list();
}

static unsigned char valid_name(char *s)
{
    unsigned char i, c;
    if (!s[0] || strlen(s) > 15) return 0;
    for (i = 0; s[i]; ++i) {
        c = s[i];
        if (c >= 'a' && c <= 'z') c -= 32;
        s[i] = c;
        if (i == 0 && !(c >= 'A' && c <= 'Z')) return 0;
        if (!((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '.')) return 0;
    }
    return 1;
}

/* returns 1 with the chosen path in pathbuf (and the file type in ftype)
   mode 0 open, 1 save, 2 save as text, 3 choose a document to print */
unsigned char __fastcall__ file_dialog(unsigned char mode)
{
    unsigned char b, i, k, n, last = 0xFF, dct = 0, r = 0;
    struct ent *e;
    fmode = mode;
    if (mode == 3) mode = fmode = 0, pmode = 1; else pmode = 0;
    if (mode) {
        strcpy(fname, docpath[0] ? strrchr(docpath, '/') + 1 : "");
        if (mode == 2 && docpath[0] && strlen(fname) < 11 && !strchr(fname, '.')) strcat(fname, ".TXT");
    }
    if (docpath[0]) {
        strcpy(browsedir, docpath);
        *strrchr(browsedir, '/') = 0;
    }
    load_dir();
    sel = scr = 0;
    ptr_hide();
    caret_hide();
    /* too large for the save-under buffer: the window is redrawn instead */
    draw_box(FC0, FC1, FY0, FY1);
    draw_dialog();
    for (;;) {
        if (sel >= nents) sel = nents ? nents - 1 : 0;
        if (sel < scr) { scr = sel; draw_list(); }
        if (sel >= scr + LROWS) { scr = sel - LROWS + 1; draw_list(); }
        e = nents ? ENT(sel) : 0;
        if (dct) --dct;
        if (wait_event() == 1) {
            k = key;
            if (k == K_ESC) break;
            if (k == K_UP) { if (sel) --sel; draw_list(); continue; }
            if (k == K_DOWN) { if (sel + 1 < nents) ++sel; draw_list(); continue; }
            if (k == K_LEFT && !mode) { go_up(); continue; }
            if (k == K_RET || (k == K_RIGHT && !mode)) { b = 0; goto button; }
            if (mode) {
                n = (unsigned char)strlen(fname);
                if (k == K_DEL || k == K_LEFT) { if (n) fname[n - 1] = 0; }
                else if (k >= 0x20 && k < 0x7F && n < 15 && !oa) {
                    if (k >= 'a' && k <= 'z') k -= 32;
                    fname[n] = k; fname[n + 1] = 0;
                }
                ptr_hide();
                draw_name();
            } else if (k >= 0x20 && k < 0x7F) {
                if (k >= 'a' && k <= 'z') k -= 32;
                for (i = 0; i < nents; ++i)
                    if ((unsigned char)(*ENT(i)).name[0] >= k) { sel = i; break; }
                draw_list();
            }
            continue;
        }
        /* mouse */
        b = hit_button();
        if (b != 0xFF) {
            while (mbtn) mouse_poll();
            if (hit_button() != b) continue;
            goto button;
        }
        if (mx >= LX0 && mx < LX1 && my >= LY0 && my < LY0 + LROWS * 11) {
            i = (my - LY0) / 11;
            if (mx >= LX1 - 18) {       /* list scroll arrows */
                if (i == 0 && scr) --scr;
                else if (i == LROWS - 1 && scr + LROWS < nents) ++scr;
                else if (i < LROWS / 2) scr = scr > LROWS ? scr - LROWS : 0;
                else if (scr + LROWS < nents) scr += LROWS;
                if (sel < scr) sel = scr;
                if (sel >= scr + LROWS) sel = scr + LROWS - 1;
                draw_list();
                while (mbtn) mouse_poll();
                continue;
            }
            if (scr + i < nents) {
                n = scr + i;
                if (n == last && dct) {     /* double click */
                    sel = n;
                    while (mbtn) mouse_poll();
                    b = 0;
                    goto button;
                }
                sel = n;
                last = n;
                dct = 60;
                if (mode && (*ENT(n)).type != 0x0F && (*ENT(n)).type != 0xFF) {
                    strcpy(fname, (*ENT(n)).name);
                    draw_name();
                }
                draw_list();
            }
        }
        while (mbtn) mouse_poll();
        continue;
button:
        if (b == 3) break;                      /* Cancel */
        if (b == 1) { go_up(); continue; }
        if (b == 2) { browsedir[0] = 0; load_dir(); sel = scr = 0; draw_folder(); draw_list(); continue; }
        /* Open / Save */
        if (e && (e->type == 0x0F || e->type == 0xFF) && (!mode || !fname[0] || !strcmp(fname, e->name))) {
            enter(e);
            continue;
        }
        if (!mode) {
            if (!e) continue;
            strcpy(pathbuf, browsedir);
            strcat(pathbuf, "/");
            strcat(pathbuf, e->name);
            ftype = e->type;
            r = 1;
            break;
        }
        if (!browsedir[0]) { beep(); continue; }
        if (!valid_name(fname)) {
            ptr_hide();
            alert("Names are 1-15 letters, digits or\nperiods, starting with a letter.", "OK", 0, 0);
            continue;
        }
        strcpy(pathbuf, browsedir);
        strcat(pathbuf, "/");
        strcat(pathbuf, fname);
        if (exists(pathbuf) && strcmp(pathbuf, docpath)) {
            ptr_hide();
            if (alert("A file with that name already\nexists. Replace it?", "Replace", "Cancel", 0)) continue;
        }
        r = 1;
        break;
    }
    ptr_hide();
    redraw_all();
    refresh();
    return r;
}

