/*
** RetroWrite II (graphics edition) - startup
*/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <apple2enh.h>
#include "rw.h"

void editor_loop(void);
unsigned char rw_init(void);
static unsigned char ramoff;
unsigned char load_parts(void);

/* Startup code is loaded into the overlay region (RW2.MAIN extends there),
   so it costs no resident memory: it runs before any overlay is loaded. */
#pragma code-name (push, "STARTCODE")
#pragma rodata-name (push, "STARTCODE")
#define ITMP  tmppath
#define IPATH pathbuf

static void __fastcall__ dir_file(char *out, const char *dir, const char *file)
{
    strcpy(out, dir);
    strcat(out, "/");
    strcat(out, file);
}

/* opens a file or folder to see that it exists */
static unsigned char __fastcall__ dir_ok(const char *path)
{
    int fd = open(path, O_RDONLY);
    if (fd < 0) return 0;
    close(fd);
    return 1;
}

static void find_progdir(void)
{
    unsigned char l = PEEK(0x280);
    char *s;
    progdir[0] = 0;
    if (l > 0 && l < 64) {
        memcpy(ITMP, (char *)0x281, l);
        ITMP[l] = 0;
        if (ITMP[0] != '/') {
            getcwd(progdir, sizeof(progdir));
            if (progdir[0] && strlen(progdir) + l + 1 < 64) {
                strcat(progdir, "/");
                strcat(progdir, ITMP);
            } else progdir[0] = 0;
        } else strcpy(progdir, ITMP);
        s = strrchr(progdir, '/');
        if (s && s != progdir) *s = 0;
        else progdir[0] = 0;
    }
    if (!progdir[0]) getcwd(progdir, sizeof(progdir));
    dir_file(ITMP, progdir, "SPELL.DICT");
    if (!dir_ok(ITMP)) {
        strcpy(ITMP, progdir);
        s = strchr(ITMP + 1, '/');
        if (s) *s = 0;
        strcat(ITMP, "/RETROWRITE");
        dir_file(IPATH, ITMP, "SPELL.DICT");
        if (dir_ok(IPATH)) strcpy(progdir, ITMP);
    }
}

static void find_docdir(void)
{
    char *s;
    strcpy(browsedir, "/DESKTOP/DOCUMENTS");
    if (dir_ok(browsedir)) return;
    strcpy(browsedir, progdir);
    s = strchr(browsedir + 1, '/');
    if (s) *s = 0;
    strcat(browsedir, "/DOCUMENTS");
    if (dir_ok(browsedir)) return;
    strcpy(browsedir, progdir);
    s = strchr(browsedir + 1, '/');
    if (s) *s = 0;
    if (!dir_ok(browsedir)) browsedir[0] = 0;
}

/* read RW2.LC into the language card and RW2.FONTS into aux memory */
static unsigned char __fastcall__ part_open(const char *name)
{
    strcpy(pathbuf, progdir);
    strcat(pathbuf, name);
    return (unsigned char)open(pathbuf, O_RDONLY);
}

unsigned char load_parts(void)
{
    unsigned char fd;
    int n;
    unsigned dst = 0xD400;
    fd = part_open("/RW2.LC");
    if (fd & 0x80) return 0;
    while ((n = read(fd, (void *)0x1400, 0x0800)) > 0) {
        (void)PEEK(0xC083);             /* LC bank 2 read/write */
        (void)PEEK(0xC083);
        memcpy((void *)dst, (void *)0x1400, n);
        (void)PEEK(0xC080);             /* back to read only */
        dst += n;
    }
    close(fd);
    if (dst < 0xD800) return 0;
    fd = part_open("/RW2.FONTS");
    if (fd & 0x80) return 0;
    n = read(fd, (void *)0x1400, 0x0900);
    close(fd);
    if (n < 0x0800) return 0;
    aux_put(0xB700, (void *)0x1400, 0x0900);
    return 1;
}

static void textmsg(const char *a)
{
    /* 80-column text message (before graphics start) */
    unsigned char i;
    gfx_off();
    POKE(0xC001, 0);
    for (i = 0; a[i]; ++i) {
        unsigned addr = 0x0628 + i / 2;        /* row 13 */
        if (i & 1) POKE(0xC054, 0); else POKE(0xC055, 0);
        POKE(addr, a[i] | 0x80);
    }
    POKE(0xC054, 0);
}

static unsigned char __fastcall__ waitkey(const char *msg)
{
    unsigned char k;
    textmsg(msg);
    while (!((k = PEEK(KBD)) & 0x80)) ;
    POKE(KBDSTRB, 0);
    return k & 0x5F;
}

/* RW2.PREFS (Startup Preferences) */
static void load_prefs(void)
{
    unsigned char k, fd;
    strcpy(IPATH, progdir);
    strcat(IPATH, "/RW2.PREFS");
    fd = open(IPATH, O_RDONLY);
    if (fd & 0x80) return;
    if (read(fd, ITMP, 73) == 73 && ITMP[0] == 'R' && ITMP[1] == 'P') {
        prt_type = ITMP[3] % 3;
        prt_slot = ITMP[4] & 7;
        if (!prt_slot) prt_slot = 1;
        prt_quality = ITMP[5] & 1;
        prt_pnum = ITMP[6] & 1;
        ruler_on = ITMP[7] & 1;
        k = ITMP[8] & 0x0F;             /* font and size only */
        if ((k & 3) == 3) k &= 0x0C;
        if ((k & 0x0C) == 0x0C) k &= 3;
        def_fmt = k;
        k = 0;
        if (ITMP[9] && dir_ok(ITMP + 9)) strcpy(browsedir, ITMP + 9);
    }
    close(fd);
}

/* coming back from BASIC.SYSTEM: put the original ProDOS quit code back */
static void restore_quit(void)
{
    unsigned char k;
    strcpy(IPATH, progdir);
    strcat(IPATH, "/RW2.QUIT");
    k = open(IPATH, O_RDONLY);
    if (k & 0x80) return;
    if (read(k, (void *)0x1400, 768) == 768) {
        (void)PEEK(0xC083);             /* language card bank 2, write */
        (void)PEEK(0xC083);
        memcpy((void *)0xD100, (void *)0x1400, 768);
        (void)PEEK(0xC080);
    }
    close(k);
    remove(IPATH);
}

/* checks, folders, language card and fonts; 0: quit */
unsigned char rw_init(void)
{
    unsigned char k, skipcheck;
    if ((PEEK(0xBF98) & 0x30) != 0x30) {
        waitkey("RetroWrite II needs an Apple IIe with 128K. Press a key.");
        return 0;
    }
    skipcheck = (PEEK(0x300) == 'R' && PEEK(0x301) == 'W' && PEEK(0x302) == 1);
    POKE(0x300, 0);
    POKE(0x302, 0);
    if (!skipcheck && (PEEK(0xBF26) != PEEK(0xBF10) || PEEK(0xBF27) != PEEK(0xBF11))) {
        iob[0x25] = iob[0x26] = 0;
        if (rd_block(0xB0, 2, iob) == 0 && (iob[0x25] | iob[0x26])) {
            for (;;) {
                k = waitkey("/RAM contains files that RetroWrite II will erase. Continue? (Y/N)");
                if (k == 'Y') break;
                if (k == 'N' || k == (K_ESC & 0x5F)) return 0;
            }
        }
    }
    find_progdir();
    find_docdir();
    load_prefs();
    restore_quit();
    ramoff = ram_disconnect();
    mouse_init();
    if (!load_parts()) {
        waitkey("RW2.LC or RW2.FONTS is missing from the RetroWrite II folder.");
        return 0;
    }
    return 1;
}
#pragma rodata-name (pop)
#pragma code-name (pop)

int main(void)
{

    if (!rw_init()) return 0;
    text_clear();
    docpath[0] = 0;
    gfx_on();
    redraw_all();
    editor_loop();

    ptr_hide();
    gfx_off();
    if (ramoff) ram_reconnect(iob);
    if (quit_now == 2 && load_ovl(OV_ACC)) basic_launch();     /* does not return */
    return 0;
}
