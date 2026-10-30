;
; RetroWrite II - DEFLATE decoder for opening .ODT files (overlay 9)
;
; Streams compressed bytes from the I/O buffer at $1000 (refilled by the C
; function inf_fill) and writes the output through a 512-byte buffer _ob,
; which C (inf_flush) empties into a file and into the 32K history window
; in aux memory $4000-$BFFF.  Matches that reach back past _ob are fetched
; from the window by C (inf_fetch) into _tmpb.
;
; Huffman decoding is canonical, one bit at a time (as in zlib's puff.c):
; tables are counts per code length (unsigned[16]) and symbols in code
; order (unsigned[]), built by C.
;
        .export         _inf_reset, _inf_codes, _inf_bits, _inf_byte, _inf_align
        .export         _inf_put, _inf_dec
        .export         _lcnt, _lsym, _dcnt, _dsym, _ob, _obfill, _tmpb
        .export         _cplen, _cpdist, _cpm, _inf_err
        .import         _inf_fill, _inf_flush, _inf_fetch

IOB     = $1000
zc      = $E0                   ; (2) counts table being decoded with
zsy     = $E2                   ; (2) symbols table
zt0     = $E4                   ; (2) scratch pointer

        .segment        "OVL9BSS"
_lcnt:  .res    32
_lsym:  .res    576
_dcnt:  .res    32
_dsym:  .res    64
_ob:    .res    512
_tmpb:  .res    316
_obfill: .res   2
_cplen: .res    2
_cpdist: .res   2
_cpm:   .res    2
_inf_err: .res  1
inrem:  .res    2
bitbuf: .res    1
bitcnt: .res    1
gbn:    .res    1
val:    .res    2
msk:    .res    2
code:   .res    2
first:  .res    2
idx:    .res    2
sum:    .res    2
tt:     .res    2
ylen:   .res    1

        .segment        "OVL9"

; void inf_reset(void)
_inf_reset:
        stz     inrem
        stz     inrem+1
        stz     bitcnt
        stz     _obfill
        stz     _obfill+1
        stz     _inf_err
        jmp     obrewind

; the store pointer goes back to the start of _ob (after C flushed it)
obrewind:
        lda     #<_ob
        sta     pst+1
        lda     #>_ob
        sta     pst+2
        rts

; ------------------------------------------------------------------------
; input

getbyte:
        lda     inrem
        ora     inrem+1
        bne     @ok
        jsr     _inf_fill               ; reads up to 512 bytes into IOB
        sta     inrem
        stx     inrem+1
        ora     inrem+1
        bne     @new
        lda     #1                      ; out of input: flag it, feed zeros
        sta     _inf_err
        lda     #0
        rts
@new:   lda     #<IOB
        sta     pld+1
        lda     #>IOB
        sta     pld+2
@ok:    lda     inrem
        bne     :+
        dec     inrem+1
:       dec     inrem
pld:    lda     IOB
        inc     pld+1
        bne     :+
        inc     pld+2
:       rts

; next bit in carry
getbit:
        lda     bitcnt
        bne     @have
        jsr     getbyte
        sta     bitbuf
        lda     #8
        sta     bitcnt
@have:  dec     bitcnt
        lsr     bitbuf
        rts

; unsigned __fastcall__ inf_bits(unsigned char n): n bits, first bit lowest
_inf_bits:
getbits:
        sta     gbn
        stz     val
        stz     val+1
        lda     #1
        sta     msk
        stz     msk+1
        lda     gbn
        beq     @done
@lp:    jsr     getbit
        bcc     @z
        lda     val
        ora     msk
        sta     val
        lda     val+1
        ora     msk+1
        sta     val+1
@z:     asl     msk
        rol     msk+1
        dec     gbn
        bne     @lp
@done:  lda     val
        ldx     val+1
        rts

; unsigned char inf_byte(void): a whole byte (stored blocks, after inf_align)
_inf_byte:
        jsr     getbyte
        ldx     #0
        rts

; void inf_align(void): drop the rest of the current byte
_inf_align:
        stz     bitcnt
        rts

; ------------------------------------------------------------------------
; output

; void __fastcall__ inf_put(unsigned char c)
_inf_put:
putbyte:
pst:    sta     _ob
        inc     pst+1
        bne     :+
        inc     pst+2
:       inc     _obfill
        bne     :+
        inc     _obfill+1
:       lda     _obfill+1
        cmp     #2                      ; 512 bytes: empty the buffer
        bne     :+
doflush:
        jsr     _inf_flush              ; sets _obfill to 0
        jmp     obrewind
:       rts

; ------------------------------------------------------------------------
; Huffman decoding with the table at (zc) / (zsy); symbol in A/X,
; $FFFF if no code matches

decode:
        stz     code
        stz     code+1
        stz     first
        stz     first+1
        stz     idx
        stz     idx+1
        ldy     #2                      ; code length 1
@lp:    sty     ylen
        jsr     getbit
        rol     code
        rol     code+1
        ldy     ylen
        clc                             ; sum = first + count[len]
        lda     first
        adc     (zc),y
        sta     sum
        iny
        lda     first+1
        adc     (zc),y
        sta     sum+1
        dey
        lda     code+1                  ; code < sum: it is this length
        cmp     sum+1
        bcc     @found
        bne     @more
        lda     code
        cmp     sum
        bcc     @found
@more:  clc                             ; idx += count[len]
        lda     idx
        adc     (zc),y
        sta     idx
        iny
        lda     idx+1
        adc     (zc),y
        sta     idx+1
        lda     sum                     ; first = sum * 2
        asl     a
        sta     first
        lda     sum+1
        rol     a
        sta     first+1
        iny
        cpy     #32
        bcc     @lp
        lda     #$FF
        tax
        rts
@found: sec                             ; symbol index = idx + code - first
        lda     code
        sbc     first
        sta     tt
        lda     code+1
        sbc     first+1
        sta     tt+1
        clc
        lda     tt
        adc     idx
        sta     tt
        lda     tt+1
        adc     idx+1
        asl     tt
        rol     a
        sta     tt+1
        clc
        lda     zsy
        adc     tt
        sta     zt0
        lda     zsy+1
        adc     tt+1
        sta     zt0+1
        ldy     #1
        lda     (zt0),y
        tax
        lda     (zt0)
        rts

uselit: lda     #<_lcnt
        sta     zc
        lda     #>_lcnt
        sta     zc+1
        lda     #<_lsym
        sta     zsy
        lda     #>_lsym
        sta     zsy+1
        rts

usedist:
        lda     #<_dcnt
        sta     zc
        lda     #>_dcnt
        sta     zc+1
        lda     #<_dsym
        sta     zsy
        lda     #>_dsym
        sta     zsy+1
        rts

; unsigned __fastcall__ inf_dec(unsigned char table): 0 literal/length, 1 distance
_inf_dec:
        cmp     #0
        bne     :+
        jsr     uselit
        jmp     decode
:       jsr     usedist
        jmp     decode

; ------------------------------------------------------------------------
; unsigned char inf_codes(void): decode one compressed block; 0 = done

_inf_codes:
@loop:  lda     _inf_err
        bne     @err
        jsr     uselit
        jsr     decode
        cpx     #0
        bne     @lenc
        jsr     putbyte                 ; a literal
        bra     @loop
@lenc:  cpx     #1
        bne     @err
        cmp     #0
        beq     @end                    ; 256: end of block
        sec
        sbc     #1                      ; length code 0..28
        cmp     #29
        bcs     @err
        tay
        lda     lbase_lo,y
        sta     _cplen
        lda     lbase_hi,y
        sta     _cplen+1
        lda     lext,y
        jsr     getbits
        clc
        adc     _cplen
        sta     _cplen
        txa
        adc     _cplen+1
        sta     _cplen+1
        jsr     usedist
        jsr     decode
        cpx     #0
        bne     @err
        cmp     #30
        bcs     @err
        tay
        lda     dbase_lo,y
        sta     _cpdist
        lda     dbase_hi,y
        sta     _cpdist+1
        lda     dext,y
        jsr     getbits
        clc
        adc     _cpdist
        sta     _cpdist
        txa
        adc     _cpdist+1
        sta     _cpdist+1
        jsr     copy
        bra     @loop
@end:   lda     _inf_err
        ldx     #0
        rts
@err:   lda     #1
        ldx     #0
        rts

; copy _cplen bytes from _cpdist back
copy:
        clc                             ; room in _ob for all of it?
        lda     _obfill
        adc     _cplen
        sta     tt
        lda     _obfill+1
        adc     _cplen+1
        cmp     #2
        bcc     @room
        bne     @fl
        lda     tt
        beq     @room
@fl:    jsr     doflush
@room:  stz     _cpm
        stz     _cpm+1
        lda     _obfill+1               ; reaches back past _ob?
        cmp     _cpdist+1
        bcc     @fetch
        bne     @part2
        lda     _obfill
        cmp     _cpdist
        bcs     @part2
@fetch: jsr     _inf_fetch              ; _cpm bytes into _tmpb (cpm <= cplen)
        lda     #<_tmpb
        sta     @pt1+1
        lda     #>_tmpb
        sta     @pt1+2
@p1:    lda     _cpm
        ora     _cpm+1
        beq     @part2
@pt1:    lda     _tmpb
        jsr     putbyte
        inc     @pt1+1
        bne     :+
        inc     @pt1+2
:       lda     _cpm
        bne     :+
        dec     _cpm+1
:       dec     _cpm
        jsr     declen
        bra     @p1
@part2: sec                             ; the rest comes from _ob itself
        lda     pst+1
        sbc     _cpdist
        sta     @pt2+1
        lda     pst+2
        sbc     _cpdist+1
        sta     @pt2+2
@p2:    lda     _cplen
        ora     _cplen+1
        beq     @done
@pt2:    lda     _ob
        jsr     putbyte
        inc     @pt2+1
        bne     :+
        inc     @pt2+2
:       jsr     declen
        bra     @p2
@done:  rts

declen: lda     _cplen
        bne     :+
        dec     _cplen+1
:       dec     _cplen
        rts

lbase_lo:
        .byte   <3,<4,<5,<6,<7,<8,<9,<10,<11,<13,<15,<17,<19,<23,<27,<31
        .byte   <35,<43,<51,<59,<67,<83,<99,<115,<131,<163,<195,<227,<258
lbase_hi:
        .byte   >3,>4,>5,>6,>7,>8,>9,>10,>11,>13,>15,>17,>19,>23,>27,>31
        .byte   >35,>43,>51,>59,>67,>83,>99,>115,>131,>163,>195,>227,>258
lext:   .byte   0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0
dbase_lo:
        .byte   <1,<2,<3,<4,<5,<7,<9,<13,<17,<25,<33,<49,<65,<97,<129,<193
        .byte   <257,<385,<513,<769,<1025,<1537,<2049,<3073,<4097,<6145
        .byte   <8193,<12289,<16385,<24577
dbase_hi:
        .byte   >1,>2,>3,>4,>5,>7,>9,>13,>17,>25,>33,>49,>65,>97,>129,>193
        .byte   >257,>385,>513,>769,>1025,>1537,>2049,>3073,>4097,>6145
        .byte   >8193,>12289,>16385,>24577
dext:   .byte   0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13
