;
; RetroWrite II - low level support routines
; Enhanced Apple IIe (65C02), 128K, ProDOS 8
;
; - Aux memory ($0800-$BFFF) access through RAMRD/RAMWRT.  These routines
;   live in the LC segment (main language card, bank 2, $D400+) so that the
;   code stays visible while main RAM $0200-$BFFF is switched out.
; - Aux language card access (dictionary cache) through ALTZP.  These live
;   in normal main RAM and use self-modified absolute addressing because
;   zero page and stack are switched while ALTZP is on.
; - Fast 80-column text output.
; - /RAM disconnect / reconnect (ProDOS 8 Technical Note #21).
;

        .setcpu         "65C02"

        .export         _aux_get, _aux_put, _aux_move, _aux_peek, _aux_poke
        .export         _alc_xfer
        .export         _ram_disconnect, _ram_reconnect, _rd_block
        .export         _key_ready

        .import         popax, popa
        .importzp       ptr1, ptr2, ptr3, tmp1

RAMRDOFF  = $C002
RAMRDON   = $C003
RAMWRTOFF = $C004
RAMWRTON  = $C005
ALTZPOFF  = $C008
ALTZPON   = $C009
PAGE2OFF  = $C054
PAGE2ON   = $C055
KBD       = $C000

DEVADR32  = $BF26
NODEV     = $BF10
DEVCNT    = $BF31
DEVLST    = $BF32

; ------------------------------------------------------------------------
; Routines that run from the language card (main LC, bank 2)
; ------------------------------------------------------------------------

        .segment        "LC"

; void __fastcall__ aux_get (void* dst_main, unsigned src_aux, unsigned n);
_aux_get:
        sta     ptr3
        stx     ptr3+1
        jsr     popax
        sta     ptr1
        stx     ptr1+1
        jsr     popax
        sta     ptr2
        stx     ptr2+1
        php
        sei
        sta     RAMRDON
        jsr     copyfwd
        sta     RAMRDOFF
        plp
        rts

; void __fastcall__ aux_put (unsigned dst_aux, const void* src_main, unsigned n);
_aux_put:
        sta     ptr3
        stx     ptr3+1
        jsr     popax
        sta     ptr1
        stx     ptr1+1
        jsr     popax
        sta     ptr2
        stx     ptr2+1
        php
        sei
        sta     RAMWRTON
        jsr     copyfwd
        sta     RAMWRTOFF
        plp
        rts

; void __fastcall__ aux_move (unsigned dst_aux, unsigned src_aux, unsigned n);
; memmove semantics inside aux memory.
_aux_move:
        sta     ptr3
        stx     ptr3+1
        jsr     popax
        sta     ptr1            ; src
        stx     ptr1+1
        jsr     popax
        sta     ptr2            ; dst
        stx     ptr2+1
        php
        sei
        sta     RAMRDON
        sta     RAMWRTON
        lda     ptr2+1          ; dst > src ?  -> copy backwards
        cmp     ptr1+1
        bne     @cmp
        lda     ptr2
        cmp     ptr1
@cmp:   bcc     @fwd
        beq     @done           ; dst == src
        jsr     copybwd
        bra     @done
@fwd:   jsr     copyfwd
@done:  sta     RAMRDOFF
        sta     RAMWRTOFF
        plp
        rts

; unsigned char __fastcall__ aux_peek (unsigned addr);
_aux_peek:
        sta     ptr1
        stx     ptr1+1
        php
        sei
        sta     RAMRDON
        lda     (ptr1)
        sta     RAMRDOFF
        plp
        ldx     #0
        rts

; void __fastcall__ aux_poke (unsigned addr, unsigned char val);
_aux_poke:
        sta     tmp1
        jsr     popax
        sta     ptr1
        stx     ptr1+1
        php
        sei
        sta     RAMWRTON
        lda     tmp1
        sta     (ptr1)
        sta     RAMWRTOFF
        plp
        rts

; copy ptr3 bytes from (ptr1) to (ptr2), ascending
copyfwd:
        ldy     #0
        ldx     ptr3+1
        beq     @part
@page:  lda     (ptr1),y
        sta     (ptr2),y
        iny
        bne     @page
        inc     ptr1+1
        inc     ptr2+1
        dex
        bne     @page
@part:  ldx     ptr3
        beq     @done
@byte:  lda     (ptr1),y
        sta     (ptr2),y
        iny
        dex
        bne     @byte
@done:  rts

; copy ptr3 bytes from (ptr1) to (ptr2), descending
copybwd:
        clc
        lda     ptr1+1
        adc     ptr3+1
        sta     ptr1+1
        clc
        lda     ptr2+1
        adc     ptr3+1
        sta     ptr2+1
        ldy     ptr3
        beq     @pages
@b1:    dey
        lda     (ptr1),y
        sta     (ptr2),y
        cpy     #0
        bne     @b1
@pages: ldx     ptr3+1
        beq     @done
@pg:    dec     ptr1+1
        dec     ptr2+1
        ldy     #$FF
@b2:    lda     (ptr1),y
        sta     (ptr2),y
        dey
        cpy     #$FF
        bne     @b2
        dex
        bne     @pg
@done:  rts

; ------------------------------------------------------------------------
; Main RAM routines
; ------------------------------------------------------------------------

        .code

; void __fastcall__ alc_xfer (void* main, unsigned lcaddr, unsigned char mode);
; Transfers 512 bytes between main RAM and the AUX language card.
; mode bit 0: 1 = main -> aux LC (put), 0 = aux LC -> main (get)
; mode bit 1: 1 = $D000 bank 1, 0 = $D000 bank 2
_alc_xfer:
        sta     xmode
        jsr     popax
        sta     xlc
        stx     xlc+1
        jsr     popax
        sta     xmn
        stx     xmn+1
        lda     xmode
        lsr     a
        bcs     @put
        lda     xlc             ; get: src = LC, dst = main
        sta     xsrc+1
        lda     xlc+1
        sta     xsrc+2
        lda     xmn
        sta     xdst+1
        lda     xmn+1
        sta     xdst+2
        bra     @go
@put:   lda     xmn             ; put: src = main, dst = LC
        sta     xsrc+1
        lda     xmn+1
        sta     xsrc+2
        lda     xlc
        sta     xdst+1
        lda     xlc+1
        sta     xdst+2
@go:    php
        sei
        sta     ALTZPON         ; no zero page / stack use from here on
        lda     xmode
        and     #2
        bne     @bank1
        lda     $C083           ; LC bank 2, read/write
        lda     $C083
        bra     @copy
@bank1: lda     $C08B           ; LC bank 1, read/write
        lda     $C08B
@copy:  ldx     #2
xpg:    ldy     #0
xsrc:   lda     $FFFF,y
xdst:   sta     $FFFF,y
        iny
        bne     xsrc
        inc     xsrc+2
        inc     xdst+2
        dex
        bne     xpg
        lda     $C080           ; back to LC bank 2 read-only (cc65 state)
        sta     ALTZPOFF
        plp
        rts

; unsigned char key_ready (void);   returns nonzero if a key is waiting
_key_ready:
        lda     KBD
        and     #$80
        ldx     #0
        rts

; unsigned char __fastcall__ rd_block (unsigned char unit, unsigned block, void* buf);
; Returns ProDOS error code (0 = ok). rd_block and ram_disconnect run only at
; startup, from the overlay area.
        .segment        "STARTCODE"
_rd_block:
        sta     rbparm+2
        stx     rbparm+3
        jsr     popax
        sta     rbparm+4
        stx     rbparm+5
        jsr     popa
        sta     rbparm+1
        jsr     $BF00
        .byte   $80             ; READ_BLOCK
        .addr   rbparm
        ldx     #0
        rts

; unsigned char ram_disconnect (void);
; Returns 1 if /RAM (slot 3, drive 2) was disconnected, 0 if not present.
_ram_disconnect:
        lda     DEVADR32
        cmp     NODEV
        bne     @present
        lda     DEVADR32+1
        cmp     NODEV+1
        bne     @present
@none:  lda     #0
        tax
        rts
@present:
        ldx     DEVCNT
@find:  lda     DEVLST,x
        and     #$F0
        cmp     #$B0
        beq     @found
        dex
        bpl     @find
        bra     @none
@found: lda     DEVLST,x
        sta     saveunit
        lda     DEVADR32
        sta     saveadr
        lda     DEVADR32+1
        sta     saveadr+1
@shift: cpx     DEVCNT
        beq     @shdone
        lda     DEVLST+1,x
        sta     DEVLST,x
        inx
        bra     @shift
@shdone:
        stz     DEVLST,x
        dec     DEVCNT
        lda     NODEV
        sta     DEVADR32
        lda     NODEV+1
        sta     DEVADR32+1
        lda     #1
        ldx     #0
        rts

        .code
; void __fastcall__ ram_reconnect (void* buf512);
; Reinstalls /RAM and asks its driver to format it.
_ram_reconnect:
        sta     $44
        stx     $45
        lda     saveunit
        beq     @done
        lda     saveadr
        sta     DEVADR32
        lda     saveadr+1
        sta     DEVADR32+1
        inc     DEVCNT
        ldx     DEVCNT
        lda     saveunit
        sta     DEVLST,x
        lda     #3              ; FORMAT
        sta     $42
        lda     saveunit
        and     #$F0
        sta     $43
        lda     $C08B           ; LC bank 1 read/write (ProDOS)
        lda     $C08B
        jsr     @call
        lda     $C080           ; back to cc65's LC state
        stz     saveunit
@done:  rts
@call:  jmp     (DEVADR32)

        .data

xmode:  .byte   0
xlc:    .word   0
xmn:    .word   0
rbparm: .byte   3, 0
        .word   0, 0
saveunit: .byte 0
saveadr:  .word 0

; unsigned char __fastcall__ scan_blk (const unsigned char* w, unsigned char wl);
; Searches prefix-compressed data block _bbuf for word w (length wl).
        .export         _scan_blk, _scan_resume, _sq, _sm
_bbuf = $1800                   ; dictionary block buffer (spell overlay)
        .importzp       tmp2, tmp3

.macro  INCP
        inc     ptr1
        bne     :+
        inc     ptr1+1
:
.endmacro

_scan_blk:
        sta     tmp1            ; wl
        jsr     popax
        sta     ptr2            ; w
        stx     ptr2+1
        lda     _scan_resume
        beq     @fresh
        lda     _sq             ; resume where the previous lookup stopped
        sta     ptr1
        lda     _sq+1
        sta     ptr1+1
        lda     _sm
        sta     tmp2
        bra     @entry
@fresh: lda     #<_bbuf
        sta     ptr1
        lda     #>_bbuf
        sta     ptr1+1
        stz     tmp2            ; m = 0
@entry: lda     ptr1
        sta     _sq
        lda     ptr1+1
        sta     _sq+1
        lda     ptr1            ; bounds check: ptr1 < bbuf+512
        cmp     #<(_bbuf+512)
        lda     ptr1+1
        sbc     #>(_bbuf+512)
        bcs     @nf
        lda     (ptr1)          ; pl
        cmp     #$FF
        beq     @nf
        INCP
        cmp     tmp2
        bcc     @nf             ; pl < m  -> past target
        bne     @skip           ; pl > m  -> still below target
        ldy     tmp2            ; i = m
@cmp:   lda     (ptr1)
        INCP
        sta     tmp3
        and     #$7F
        cpy     tmp1
        bcs     @nf             ; dictionary word longer than target
        cmp     (ptr2),y
        beq     @same
        bcs     @nf             ; c > w[i]
        sty     tmp2            ; c < w[i]: m = i
        lda     tmp3
        bmi     @entry
        bra     @skip
@same:  iny
        lda     tmp3
        bpl     @cmp
        cpy     tmp1
        beq     @found
        sty     tmp2            ; dictionary word is a prefix of target
        bra     @entry
@skip:  lda     (ptr1)
        INCP
        tax
        bpl     @skip
        bra     @entry
@found: lda     ptr1            ; resume after this word, cur == target
        sta     _sq
        lda     ptr1+1
        sta     _sq+1
        lda     tmp1
        sta     _sm
        lda     #1
        ldx     #0
        rts
@nf:    lda     tmp2            ; resume at the first entry > target
        sta     _sm
        lda     #0
        tax
        rts

        .data
_scan_resume:   .byte   0
_sq:            .word   0
_sm:            .byte   0
        .code

; signed char __fastcall__ cmpw (const unsigned char* a, unsigned char al,
;                               const unsigned char* b, unsigned char bl);
; Lexical compare of two length-counted byte strings: -1, 0, 1
        .export         _cmpw
_cmpw:
        sta     tmp2            ; bl
        jsr     popax
        sta     ptr2            ; b
        stx     ptr2+1
        jsr     popa
        sta     tmp1            ; al
        jsr     popax
        sta     ptr1            ; a
        stx     ptr1+1
        ldy     #0
@lp:    cpy     tmp1
        beq     @aend
        cpy     tmp2
        beq     @gt             ; b ended first, a longer
        lda     (ptr1),y
        cmp     (ptr2),y
        bne     @diff
        iny
        bra     @lp
@aend:  cpy     tmp2
        beq     @eq
@lt:    lda     #$FF
        tax
        rts
@diff:  bcc     @lt
@gt:    lda     #1
        ldx     #0
        rts
@eq:    lda     #0
        tax
        rts

        .segment        "OVL1"         ; used only by the file overlay
; unsigned __fastcall__ filt (unsigned n);
; In-place filter of _iob[0..n-1] for loading: strip bit 7, LF/CRLF -> CR,
; keep CR, TAB and $20-$7E. Returns new length. State in _fprev.
        .export         _filt, _fprev
_iob = $1000                    ; I/O buffer (rw.h)
_filt:
        sta     ptr3            ; n
        stx     ptr3+1
        sta     fn
        stx     fn+1
        stz     fdrop
        stz     fdrop+1
        lda     #<_iob
        sta     ptr1
        sta     ptr2
        lda     #>_iob
        sta     ptr1+1
        sta     ptr2+1
        ldx     _fprev
@page:  lda     ptr3+1          ; full page left?
        beq     @part
        dec     ptr3+1
        stz     tmp1            ; 256 bytes
        bra     @go
@part:  lda     ptr3
        beq     @done
        sta     tmp1
        stz     ptr3
@go:    ldy     #0
@in:    lda     (ptr1),y
        and     #$7F
        cmp     #$20
        bcc     @ctrl
        cmp     #$7F
        beq     @drop
        ldx     #0
@st:    sta     (ptr2),y
@nx:    iny
        cpy     tmp1
        bne     @in
        inc     ptr1+1
        inc     ptr2+1
        bra     @page
@ctrl:  cmp     #$0A
        bne     @nlf
        cpx     #$0D
        beq     @lfd
        lda     #$0D
@nlf:   cmp     #$0D
        beq     @cr
        cmp     #$09
        bne     @drop
        ldx     #0
        bra     @st
@cr:    tax
        bra     @st
@lfd:   ldx     #$0A
        bra     @drop2
@drop:  ldx     #0
@drop2: lda     ptr2            ; dest pointer back by one
        bne     :+
        dec     ptr2+1
:       dec     ptr2
        inc     fdrop
        bne     @nx
        inc     fdrop+1
        bra     @nx
@done:  stx     _fprev
        sec
        lda     fn
        sbc     fdrop
        pha
        lda     fn+1
        sbc     fdrop+1
        tax
        pla
        rts

_fprev: .byte   0
fn:     .word   0
fdrop:  .word   0
        .code

; ------------------------------------------------------------------------
; unsigned char __fastcall__ shift_from (unsigned char j0);
; After an edit (core.c edited()): new screen line j0 starts at L_next with
; format L_nf. If an old line k (k != j0) started there (positions after the
; edit adjusted by ed_delta), the old layout from k on is reused for j0 on.
; Returns 1 if done, 0 if no old line matches.
        .export         _shift_from
        .import         _ls, _lf, _lh, _la, _ly, _ldirty, _olds, _oldf
        .import         _nl, _below_dirty, _layout_ok, _L_next, _L_nf
        .import         _ed_p, _ed_delta, _lp, _lq, _lns, _lw, _oldp

SMAXL   = 24
SBOTY   = 192

        .code
_shift_from:
        sta     sj0
        ldx     #1
@srch:  cpx     _nl
        bcs     @fail
        txa
        asl     a
        tay
        jsr     oldq            ; sq = adjusted olds[y/2]
        lda     sq
        cmp     _L_next
        bne     @next
        lda     sq+1
        cmp     _L_next+1
        bne     @next
        lda     _oldf,x
        cmp     _L_nf
        bne     @next
        cpx     sj0
        bne     @found
@next:  inx
        bra     @srch
@fail:  lda     #0
        tax
        rts

@found: stx     sk
        lda     _nl             ; n = nl - k, at most MAXL - j0
        sec
        sbc     sk
        sta     sn
        clc
        adc     sj0
        cmp     #SMAXL + 1
        bcc     :+
        lda     #SMAXL
        sec
        sbc     sj0
        sta     sn
:       lda     sk              ; heights and ascents: overlapping move
        cmp     sj0
        bcc     @desc
        ldy     #0
@asc:   cpy     sn
        bcs     @hdone
        jsr     mvhla
        iny
        bra     @asc
@desc:  ldy     sn
        beq     @hdone
@dl:    dey
        jsr     mvhla
        cpy     #0
        bne     @dl
@hdone: ldy     #0              ; starts and formats, j = 0..n
@sl:    tya
        clc
        adc     sk
        tax
        lda     _oldp,x
        pha
        lda     _oldf,x
        sta     stmp
        txa
        asl     a
        phy
        tay
        jsr     oldq
        ply
        tya
        clc
        adc     sj0
        tax
        pla
        sta     _lp,x
        lda     stmp
        sta     _lf,x
        txa
        asl     a
        tax
        lda     sq
        sta     _ls,x
        lda     sq+1
        sta     _ls+1,x
        iny
        cpy     sn
        bcc     @sl
        beq     @sl
        ldx     sj0             ; y = ly[j0-1] + lh[j0-1]
        dex
        lda     _ly,x
        clc
        adc     _lh,x
        sta     sy
        lda     sj0
        clc
        adc     sn
        sta     send
        ldx     sj0
@pl:    cpx     send
        bcs     @pd
        lda     sy
        cmp     #SBOTY
        bcs     @pd
        sta     _ly,x
        clc
        adc     _lh,x
        sta     sy
        lda     #1
        sta     _ldirty,x
        inx
        bra     @pl
@pd:    stx     _nl
        lda     sy
        cmp     #SBOTY
        bcc     :+
        lda     #SBOTY
:       sta     _ly,x
        lda     #1
        sta     _below_dirty
        lda     sy              ; more lines needed below: full layout
        cmp     #SBOTY
        bcs     @ok
        cpx     #SMAXL
        bcs     @ok
        txa
        asl     a
        tay
        lda     _ls,y
        and     _ls+1,y
        cmp     #$FF
        beq     @ok
        stz     _layout_ok
@ok:    lda     #1
        ldx     #0
        rts

; move lh/la/lq/lns/lw from index k+y to j0+y
mvhla:  tya
        clc
        adc     sk
        tax
        lda     _lh,x
        pha
        lda     _la,x
        pha
        lda     _lq,x
        pha
        lda     _lns,x
        pha
        txa
        asl     a
        tax
        lda     _lw,x
        pha
        lda     _lw+1,x
        pha
        tya
        clc
        adc     sj0
        asl     a
        tax
        pla
        sta     _lw+1,x
        pla
        sta     _lw,x
        txa
        lsr     a
        tax
        pla
        sta     _lns,x
        pla
        sta     _lq,x
        pla
        sta     _la,x
        pla
        sta     _lh,x
        rts

; sq = olds[Y/2], plus ed_delta if it is a position after ed_p
oldq:   lda     _olds,y
        sta     sq
        lda     _olds+1,y
        sta     sq+1
        and     sq
        cmp     #$FF            ; NOLINE
        beq     @r
        lda     _ed_p           ; ed_p < sq ?
        cmp     sq
        lda     _ed_p+1
        sbc     sq+1
        bcs     @r
        lda     sq
        clc
        adc     _ed_delta
        sta     sq
        lda     sq+1
        adc     _ed_delta+1
        sta     sq+1
@r:     rts

        .bss
sj0:    .res    1
sk:     .res    1
sn:     .res    1
sq:     .res    2
stmp:   .res    1
sy:     .res    1
send:   .res    1
