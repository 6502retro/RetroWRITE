/*
** RetroWrite II - spelling checker (graphics edition)
*/
#pragma code-name (push, "OVL2")
#pragma rodata-name (push, "OVL2")
#pragma data-name (push, "OVL2")
#pragma bss-name (push, "OVL2BSS")

#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <apple2enh.h>
#include "rw.h"

static unsigned char __fastcall__ lowc(unsigned char c)
{
    return (c >= 'A' && c <= 'Z') ? c + 32 : c;
}
static unsigned char __fastcall__ upc(unsigned char c)
{
    return (c >= 'a' && c <= 'z') ? c - 32 : c;
}

#include "spell_dict.inc"

/* ---------------------------------------------------------------------- */
/* Word reader over the document (format markers are transparent)          */

static unsigned char rdn;
#define rdbuf TB                        /* 256-byte scratch, also used for drawing */
static unsigned char __fastcall__ rd(unsigned p)
{
    if (p - rdbase >= rdn || rdbase == 0xFFFF) {
        rdbase = p;
        rdn = (tlen - p) > 128 ? 128 : (unsigned char)(tlen - p);
        fetch(p, rdbuf, rdn);
    }
    return rdbuf[p - rdbase];
}


#pragma bss-name (push, "TXTBSS1")   /* text page $0400-$0477 */
static unsigned char sw[40];            /* normalized word */
static char tok[40];                    /* word as written */
static unsigned sgx0[9], sgx1[9];
#pragma bss-name (pop)
static unsigned char swl;
static unsigned char tokl;
static unsigned wstart, wend;

static unsigned char __fastcall__ next_word(unsigned p, unsigned to)
{
    unsigned q;
    unsigned char c, digit, s, e;
    for (;;) {
        while (p < to && (((c = rd(p)) & 0x80) || !isword(c))) ++p;
        if (p >= to) return 0;
        q = p;
        digit = 0;
        tokl = 0;
        while (q < tlen) {
            c = rd(q);
            if (c & 0x80) { ++q; continue; }
            if (!isword(c)) break;
            if (c >= '0' && c <= '9') digit = 1;
            if (tokl < 39) tok[tokl++] = c;
            ++q;
        }
        tok[tokl] = 0;
        wstart = p;
        wend = q;
        p = q;
        if (digit || tokl > 31) continue;
        for (s = 0; s < tokl; ++s) sw[s] = lowc(tok[s]);
        s = 0;
        e = tokl;
        while (s < e && sw[s] == '\'') ++s;
        while (e > s && sw[e - 1] == '\'') --e;
        if (e - s >= 3 && sw[e - 2] == '\'' && sw[e - 1] == 's') e -= 2;
        swl = e - s;
        if (s) memmove(sw, sw + s, swl);
        if (swl < 2) continue;
        return 1;
    }
}

static unsigned char word_ok(void)
{
    if (swl > 31) return 0;
    if (pool_find(sw, swl)) return 1;
    return dict_find(sw, swl);
}

#include "spell_sugg.inc"

static void __fastcall__ match_case(char *rep)
{
    unsigned char i, allup = 1;
    for (i = 0; i < tokl; ++i)
        if (tok[i] >= 'a' && tok[i] <= 'z') { allup = 0; break; }
    if (allup && tokl > 1) {
        for (i = 0; rep[i]; ++i) rep[i] = upc(rep[i]);
    } else if (tok[0] >= 'A' && tok[0] <= 'Z') {
        rep[0] = upc(rep[0]);
    }
}

/* replace [wstart, wend) keeping the format that follows; returns delta */
static int __fastcall__ replace_word(const char *rep)
{
    unsigned n = strlen(rep), old = wend - wstart;
    unsigned char fend = fmt_at(wend), m;
    raw_delete(wstart, old);
    raw_insert(wstart, (const unsigned char *)rep, n);
    wend = wstart + n;
    if (fmt_at(wend) != fend) {
        m = 0x80 | fend;
        raw_insert(wend, &m, 1);
        ++wend;
    }
    disp_shift(wstart, -(int)old);
    disp_shift(wstart, wend - wstart);
    rdbase = 0xFFFF;
    text_moved();
    return (int)(wend - wstart) - (int)old;
}

/* ---------------------------------------------------------------------- */
/* Dialog                                                                  */

#define SC0 3
#define SC1 76
#define SX0 (SC0 * 7)
#define SX1 (SC1 * 7 + 7)
#define SY0 122
#define SY1 189

static const char *const sbtn[6] = { "Skip", "Replace", "Ignore All", "Learn", "Suggest", "Stop" };

static void spell_dialog(void)
{
    unsigned char i;
    unsigned x;
    ptr_hide();
    scr_save(SC0, SC1, SY0, SY1);
    draw_box(SC0, SC1, SY0, SY1);
    lb_clear(11);
    ui_text(SX0 + 14, 8, 0, "Not in the dictionary:  ");
    ui_text(gx, 8, F_BOLD, tok);
    present(SY0 + 6, 11, SC0 + 1, SC1 - 1);
    nbtn = 6;
    x = SX0 + 12;
    for (i = 0; i < 6; ++i) {
        blabel[i] = sbtn[i];
        bx0[i] = x;
        bx1[i] = x + 74;
        by[i] = SY1 - 22;
        draw_button(bx0[i], bx1[i], by[i], sbtn[i], i == 0);
        x += 82;
    }
}

/* suggestion "buttons" drawn as a row of words */
static void show_suggestions(void)
{
    unsigned char i;
    ptr_hide();
    lb_clear(11);
    if (!nsugg) {
        ui_text(SX0 + 14, 8, F_ITAL, "No suggestions.");
    } else {
        ui_text(SX0 + 14, 8, 0, "Suggestions:");
        gx += 8;
        for (i = 0; i < nsugg; ++i) {
            sgx0[i] = gx;
            ui_text(gx, 8, F_UND, sugg[i]);
            sgx1[i] = gx;
            gx += 10;
            if (gx > SX1 - 60) { nsugg = i + 1; break; }
        }
    }
    present(SY0 + 20, 11, SC0 + 1, SC1 - 1);
}

#pragma bss-name (push, "TXTBSS2")   /* text page $0480-$04F7 */
static char rep[32], msg[48];
#pragma bss-name (pop)

void cmd_spell(void)
{
    unsigned p, start, endp;
    unsigned char k, i, wrapped = 0, stop = 0, sugg_shown;
    unsigned unknown = 0;
    int d;

    user_load();                        /* before the dictionary: one file at a time */
    if (!dict_open()) return;
    dict_stale();
    spell_err = 0;
    rdbase = 0xFFFF;
    start = cur < anc ? cur : anc;
    while (start > 1 && isword(tch(start - 1))) --start;
    if (skip_fwd(start) >= tlen) start = 1;     /* at the end: check it all */
    p = start;
    endp = tlen;
    pshape = 2;
    ptr_hide();
    ptr_show();
    for (;;) {
        while (next_word(p, endp)) {
            p = wend;
            if ((PEEK(KBD) & 0xFF) == (0x80 | K_ESC)) {
                POKE(KBDSTRB, 0);
                stop = 1;
                break;
            }
            if (word_ok()) continue;
            if (spell_err) break;
            ++unknown;
            /* show the word selected, then the dialog */
            {
                unsigned olo = cur < anc ? cur : anc, ohi = cur < anc ? anc : cur;
                anc = wstart;
                cur = wend;
                sel_changed(olo, ohi);      /* only the changed lines redraw */
                scroll_into(SY0 - 2);       /* keep the word above the dialog */
            }
            pshape = 0;
            refresh();
            spell_dialog();
            sugg_shown = 0;
            for (;;) {
                k = run_buttons(1);
                if (k == 0xFD && sugg_shown) {          /* click on a suggestion? */
                    for (i = 0; i < nsugg; ++i)
                        if (my >= SY0 + 20 && my < SY0 + 31 && mx >= sgx0[i] && mx < sgx1[i]) break;
                    if (i < nsugg) {
                        strcpy(rep, sugg[i]);
                        match_case(rep);
                        k = 0xF0;
                    } else continue;
                } else if (k >= 0xF0) {
                    if ((key | 0x20) == 'g') { k = 4; goto suggest_k; }
                    if (key >= '1' && key <= '9' && sugg_shown && key - '1' < nsugg) {
                        strcpy(rep, sugg[key - '1']);
                        match_case(rep);
                        k = 0xF0;
                    } else continue;
                }
                if (k == 0xF0) {
                    ptr_hide();
                    scr_restore();
                    d = replace_word(rep);
                    endp += d;
                    p = wend;
                    break;
                }
                if (k == 0) break;                          /* Skip */
                if (k == 5) { stop = 1; break; }            /* Stop */
                if (k == 2) { pool_add(sw, swl, 1); break; }
                if (k == 3) {
                    if (!pool_add(sw, swl, 0) || !user_save()) {
                        ptr_hide();
                        scr_restore();
                        show_msg("Could not save USER.DICT.");
                        spell_dialog();
                        continue;
                    }
                    break;
                }
                if (k == 1) {
                    strcpy(rep, tok);
                    ptr_hide();
                    scr_restore();
                    if (ask_text("Replace with:", rep, 30) && rep[0]) {
                        d = replace_word(rep);
                        endp += d;
                        p = wend;
                        break;
                    }
                    spell_dialog();
                    continue;
                }
                if (k == 4) {
suggest_k:
                    pshape = 2;
                    ptr_hide();
                    ptr_show();
                    dict_stale();
                    suggest();
                    pshape = 0;
                    show_suggestions();
                    sugg_shown = 1;
                    continue;
                }
            }
            if (k != 0xF0 && k != 1) { ptr_hide(); scr_restore(); }
            dict_stale();
            pshape = 2;
            if (stop) break;
            if (endp > tlen) endp = tlen;
        }
        if (stop || spell_err) break;
        if (!wrapped && start > 1) {
            pshape = 0;
            if (alert("The end of the document was reached.\nContinue checking from the top?", "Continue", "Stop", 0) == 0) {
                wrapped = 1;
                dict_stale();
                endp = start;
                p = 1;
                pshape = 2;
                continue;
            }
            stop = 1;                   /* stopped: no "complete" message */
        }
        break;
    }
    pshape = 0;
    ptr_hide();
    invalidate_layout();
    scroll_to_caret();
    set_all_dirty();
    refresh();
    dict_close();                       /* the overlay may be unloaded */
    if (spell_err) {
        show_msg("Error reading SPELL.DICT.");
    } else if (!stop) {
        if (!unknown) strcpy(msg, "Spelling check complete.\nNo unknown words found.");
        else {
            strcpy(msg, "Spelling check complete.\n");
            utoa(unknown, msg + strlen(msg), 10);
            strcat(msg, unknown == 1 ? " word was not in the dictionary." : " words were not in the dictionary.");
        }
        show_msg(msg);
    }
}
