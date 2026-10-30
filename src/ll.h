/* low level routines (lowlevel.s) */
void __fastcall__ aux_get(void *dst, unsigned src, unsigned n);
void __fastcall__ aux_put(unsigned dst, const void *src, unsigned n);
void __fastcall__ aux_move(unsigned dst, unsigned src, unsigned n);
unsigned char __fastcall__ aux_peek(unsigned addr);
void __fastcall__ aux_poke(unsigned addr, unsigned char v);
void __fastcall__ alc_xfer(void *mainbuf, unsigned lcaddr, unsigned char mode);
unsigned char key_ready(void);
unsigned char __fastcall__ rd_block(unsigned char unit, unsigned block, void *buf);
unsigned char ram_disconnect(void);
void __fastcall__ ram_reconnect(void *buf);
unsigned __fastcall__ filt(unsigned n);
extern unsigned char fprev;
signed char __fastcall__ cmpw(const unsigned char *a, unsigned char al, const unsigned char *b, unsigned char bl);
unsigned char __fastcall__ scan_blk(const unsigned char *w, unsigned char wl);
extern unsigned char scan_resume, sm;
