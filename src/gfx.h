/* RetroWrite II graphics engine interface (gfx.s) */
extern unsigned gx;
extern unsigned char gbase, gfmt;
extern unsigned mx;
extern unsigned char my, mbtn, mslot, pshape, ptr_on;
void gfx_on(void);
void gfx_off(void);
void __fastcall__ lb_clear(unsigned char nrows);
void __fastcall__ lb_fill(unsigned x0, unsigned x1, unsigned char r0, unsigned char r1, unsigned char mode);
unsigned char __fastcall__ draw_char(unsigned char ch);
unsigned char __fastcall__ char_width(unsigned char ch);
void __fastcall__ present(unsigned char y, unsigned char nrows, unsigned char c0, unsigned char c1);
void __fastcall__ scr_copy(unsigned char sy, unsigned char dy, unsigned char n, unsigned char c0, unsigned char c1);
void __fastcall__ scr_save(unsigned char c0, unsigned char c1, unsigned char y0, unsigned char y1);
void scr_restore(void);
void __fastcall__ caret_xor(unsigned x, unsigned char y0, unsigned char y1);
unsigned char mouse_init(void);
void mouse_poll(void);
void ptr_show(void);
void ptr_hide(void);
#define LB_CLR  0
#define LB_SET  1
#define LB_XOR  2
#define LB_GRAY 3
#define LB_DIM  4
