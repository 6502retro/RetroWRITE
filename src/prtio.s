;
; RetroWrite II - printer output (part of the print overlay, RW2.OVL5)
;
; prt_open(slot)   finds the printer interface in the slot:
;                    Pascal 1.1 firmware (Super Serial Card, Grappler,
;                    most later cards) - bytes go out unchanged, or
;                    the Apple Parallel Interface card - written to its port.
;                  returns 0 if a printer card was found, else 1
; prt_byte(c)      sends one byte
; prt_band()       sends one graphics band: line-buffer rows pb_row..+3,
;                  pixels pb_x0..pb_x1-1, each screen row printed as two dots
;                  (ImageWriter: bit 0 at the top; Epson: bit 7 at the top)
;
        .setcpu         "65C02"
        .export         _prt_open, _prt_byte, _prt_band
        .export         _pb_row, _pb_x0, _pb_x1, _pb_dots
        .import         _div7
        .importzp       ptr1, ptr2, ptr3, ptr4, tmp1, tmp2, tmp3

LB      = $1400

        .segment        "OVL5"

; unsigned char __fastcall__ prt_open (unsigned char slot)
_prt_open:
        and     #7
        sta     pslot
        ora     #$C0
        sta     ptr1+1
        stz     ptr1
        asl     a               ; slot * 16 for the I/O port
        asl     a
        asl     a
        asl     a
        and     #$70
        sta     pport
        lda     ptr1+1
        sta     pcn
        ldy     #$05            ; Pascal 1.1 signature
        lda     (ptr1),y
        cmp     #$38
        bne     p_notpas
        ldy     #$07
        lda     (ptr1),y
        cmp     #$18
        bne     p_notpas
        ldy     #$0B
        lda     (ptr1),y
        cmp     #$01
        bne     p_notpas
        ldy     #$0F            ; write entry
        lda     (ptr1),y
        sta     pwrite+1
        lda     ptr1+1
        sta     pwrite+2
        ldy     #$0D            ; init entry
        lda     (ptr1),y
        sta     pinit+1
        lda     ptr1+1
        sta     pinit+2
        lda     #1
        sta     pmode
        jsr     romon
pinit:  jsr     $FFFF           ; patched: Pascal init
        jsr     romoff
        lda     #0
        tax
        rts
p_notpas:
        ldy     #0              ; Apple Parallel Interface: CLC / BCS / SEC
        lda     (ptr1),y
        cmp     #$18
        bne     p_none
        iny
        lda     (ptr1),y
        cmp     #$B0
        bne     p_none
        lda     #2
        sta     pmode
        lda     #0
        tax
        rts
p_none:  stz     pmode
        lda     #1
        ldx     #0
        rts

; call firmware with the ROM (not the language card) at $D000-$FFFF
romon:  php
        pla
        sta     pflags
        sei
        bit     $C082
        ldx     pcn
        txa
        asl     a
        asl     a
        asl     a
        asl     a
        tay                     ; Y = $n0, X = $Cn
        rts
romoff: bit     $C080           ; language card bank 2, read (cc65 state)
        lda     pflags
        pha
        plp
        rts

; void __fastcall__ prt_byte (unsigned char c)
_prt_byte:
        ldx     pmode
        cpx     #1
        bne     p_par
        pha
        jsr     romon
        pla
pwrite: jsr     $FFFF           ; patched: Pascal write
        jmp     romoff
p_par:   cpx     #2
        bne     p_rts
        ldx     pport
        sta     $C080,x
        ldx     #12             ; give the card time
:       dex
        bne     :-
p_rts:   rts

; void prt_band (void)
_prt_band:
        lda     _pb_row         ; row pointers ptr1..ptr4 = LB + (row+r)*80
        jsr     rowptr
        sta     ptr1
        stx     ptr1+1
        inc     _pb_row
        lda     _pb_row
        jsr     rowptr
        sta     ptr2
        stx     ptr2+1
        inc     _pb_row
        lda     _pb_row
        jsr     rowptr
        sta     ptr3
        stx     ptr3+1
        inc     _pb_row
        lda     _pb_row
        jsr     rowptr
        sta     ptr4
        stx     ptr4+1
        inc     _pb_row
        lda     _pb_x0          ; column and bit of the first pixel
        ldx     _pb_x0+1
        jsr     _div7
        sta     tmp1            ; byte column
        lda     #1
:       cpx     #0
        beq     :+
        asl     a
        dex
        bra     :-
:       sta     tmp2            ; bit mask
        lda     _pb_x1          ; pixel count
        sec
        sbc     _pb_x0
        sta     pcount
        lda     _pb_x1+1
        sbc     _pb_x0+1
        sta     pcount+1
@col:   lda     pcount
        ora     pcount+1
        beq     @done
        stz     tmp3
        ldy     tmp1
        lda     (ptr1),y
        and     tmp2
        beq     :+
        lda     _pb_dots+0
        jsr     addd
:       lda     (ptr2),y
        and     tmp2
        beq     :+
        lda     _pb_dots+1
        jsr     addd
:       lda     (ptr3),y
        and     tmp2
        beq     :+
        lda     _pb_dots+2
        jsr     addd
:       lda     (ptr4),y
        and     tmp2
        beq     :+
        lda     _pb_dots+3
        jsr     addd
:       lda     tmp3
        jsr     _prt_byte
        asl     tmp2            ; next pixel
        lda     tmp2
        cmp     #$80
        bcc     :+
        lda     #1
        sta     tmp2
        inc     tmp1
:       lda     pcount
        bne     :+
        dec     pcount+1
:       dec     pcount
        bra     @col
@done:  rts

addd:   ora     tmp3
        sta     tmp3
        rts

; A = row -> A/X = LB + row * 80
rowptr: sta     prow
        stz     prow+1
        asl     prow            ; *16
        rol     prow+1
        asl     prow
        rol     prow+1
        asl     prow
        rol     prow+1
        asl     prow
        rol     prow+1
        lda     prow            ; *16 + *64 = *80
        sta     tmp3
        ldx     prow+1
        asl     prow
        rol     prow+1
        asl     prow
        rol     prow+1
        clc
        lda     prow
        adc     tmp3
        sta     prow
        txa
        adc     prow+1
        tax
        lda     prow
        clc
        adc     #<LB
        pha
        txa
        adc     #>LB
        tax
        pla
        rts


        .segment        "OVL5BSS"
_pb_row:   .res 1
_pb_x0:    .res 2
_pb_x1:    .res 2
_pb_dots:  .res 4            ; dot pairs for rows 0-3 (set by the C side)
pslot:  .res    1
pport:  .res    1
pcn:    .res    1
pmode:  .res    1
pflags: .res    1
pcount: .res    2
prow:   .res    2
