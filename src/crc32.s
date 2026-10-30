;
; RetroWrite II - CRC-32 for the ODT (ZIP) writer, overlay 11
;
; void crc_init(void)                          builds the table (1K)
; void __fastcall__ crc_update(const void *buf, unsigned n)
; unsigned long crcv                           running value (start ~0, end ~crc)
;
        .export         _crc_init, _crc_update, _crcv
        .importzp       ptr1, ptr2, tmp1
        .import         popax

; the table lives in the line buffer ($1400-$17FF), which nothing draws
; into while a document is being saved
t0      = $1400
t1      = $1500
t2      = $1600
t3      = $1700
        .segment        "OVLBBSS"
_crcv:  .res    4
c0:     .res    1
c1:     .res    1
c2:     .res    1
c3:     .res    1

        .segment        "OVLB"

_crc_init:
        ldx     #0
@ent:   stx     c0
        stz     c1
        stz     c2
        stz     c3
        ldy     #8
@bit:   lsr     c3
        ror     c2
        ror     c1
        ror     c0
        bcc     @nx
        lda     c3
        eor     #$ED
        sta     c3
        lda     c2
        eor     #$B8
        sta     c2
        lda     c1
        eor     #$83
        sta     c1
        lda     c0
        eor     #$20
        sta     c0
@nx:    dey
        bne     @bit
        lda     c0
        sta     t0,x
        lda     c1
        sta     t1,x
        lda     c2
        sta     t2,x
        lda     c3
        sta     t3,x
        inx
        bne     @ent
        rts

; buf in ptr1, count in ptr2
_crc_update:
        sta     ptr2
        stx     ptr2+1
        jsr     popax
        sta     ptr1
        stx     ptr1+1
        ldy     #0
@lp:    lda     ptr2
        ora     ptr2+1
        beq     @done
        lda     (ptr1),y
        eor     _crcv
        tax
        lda     _crcv+1
        eor     t0,x
        sta     _crcv
        lda     _crcv+2
        eor     t1,x
        sta     _crcv+1
        lda     _crcv+3
        eor     t2,x
        sta     _crcv+2
        lda     t3,x
        sta     _crcv+3
        iny
        bne     :+
        inc     ptr1+1
:       lda     ptr2
        bne     :+
        dec     ptr2+1
:       dec     ptr2
        bra     @lp
@done:  rts
