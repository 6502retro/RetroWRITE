;
; RetroWrite II - memory setup
; * C stack moved to $1000-$13FF (low memory), freeing main RAM for code
; * ProDOS file buffer: one, at $0800 (only one file is open at a time)
;
        .setcpu         "65C02"
        .constructor    setstack, 31
        .export         iobuf_alloc, iobuf_free, _iob_limit
        .import         incsp2, popptr1
        .importzp       c_sp, ptr1

        .include        "errno.inc"

        .segment        "ONCE"
setstack:
        lda     #<$1400
        sta     c_sp
        lda     #>$1400
        sta     c_sp+1
        rts

        .code
; int iobuf_alloc (void** memptr, size_t size, size_t align)  (cc65 internal)
iobuf_alloc:
        jsr     incsp2
        jsr     popptr1
        ldx     #0
:       lda     used,x
        beq     :+
        inx
        cpx     _iob_limit      ; one buffer: $0C00-$0FFF holds the word pool,
        bcc     :-              ; which the ODT overlays may borrow for a second
        lda     #ENOMEM
        rts
:       dec     used,x
        txa
        asl     a
        asl     a
        adc     #>$0800
        iny
        sta     (ptr1),y
        dey
        tya
        sta     (ptr1),y
        rts

; void iobuf_free (void* mem)
iobuf_free:
        txa
        sec
        sbc     #>$0800
        lsr     a
        lsr     a
        tax
        stz     used,x
        rts

        .data
used:   .byte   0, 0
_iob_limit: .byte 1
