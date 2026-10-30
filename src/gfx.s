;
; RetroWrite II - double hi-res graphics engine (65C02)
;
; Screen: 560 x 192 monochrome double hi-res, page 1 ($2000-$3FFF main+aux).
; Drawing happens in a line buffer LB (up to 32 rows x 80 bytes) laid out
; like a DHGR scanline: byte c holds pixels 7c..7c+6 in bits 0-6 (bit 0 =
; leftmost). A set bit means "ink" (black); present() inverts while copying
; so paper is white.
;
        .setcpu         "65C02"

        .export         _lb_clear, _lb_fill, _draw_char, _char_width
        .export         _present, _scr_copy, _scr_save, _scr_restore
        .export         _gfx_on, _gfx_off, _div7, _caret_xor
        .export         _line_engine, _lb_cmds, _lbx
        .export         _lm_n, _lm_tx, _lm_s0, _lm_s1, _lm_x0, _lm_x1
        .export         _lm_i, _lm_nf, _lm_asc, _lm_desc, _lm_cr
        .export         _lm_pa, _lm_np, _lm_ns, _lm_jn, _lm_jx, _lm_jr, _lm_w
        .export         _wrapx, _tabbase, _tabs
        .export         _gx, _gbase, _gfmt, _fonts
        .export         _mouse_init, _mouse_poll, _mx, _my, _mbtn, _mslot
        .export         _ptr_show, _ptr_hide, _pshape, _ptr_on
        .import         popax, popa
        .import         _font_sans, _font_serif, _font_mono
        .import         _fbm_sans, _fbm_serif, _fbm_mono

LB      = $1400                 ; line buffer, 32 rows x 80 bytes ($1400-$1DFF)
LBROWS  = 32
SAVEBUF = $0800                 ; save-under buffer in AUX memory ($0800-$1FFF)

PAGE2OFF = $C054
PAGE2ON  = $C055

; private zero page ($E0-$FF)
zrow    = $E0                   ; (2) line buffer row pointer
zv      = $E2                   ; (6) shifted value
zn      = $E8                   ; bytes in zv
zcol    = $E9                   ; byte column
zs      = $EA                   ; bit shift 0..6
zfp     = $EB                   ; (2) font pointer
zgp     = $ED                   ; (2) glyph bitmap pointer
zscr    = $EF                   ; (2) screen pointer
zt      = $F1
zt2     = $F2
zrowi   = $F3
zscale  = $F4
zw      = $F5
zgr     = $F6
zd0     = $F7                   ; glyph row data (3 bytes)
zd1     = $F8
zd2     = $F9
zrep    = $FA
zcnt    = $FB
zsave   = $FC                   ; (2)
zb0     = $FE                   ; save/restore: first byte offset
zb1     = $FF                   ; save/restore: last byte offset

        .data
_gx:    .word   0               ; pen x (pixels)
_gbase: .byte   0               ; baseline row in the line buffer
_gfmt:  .byte   0               ; bits 0-1 font, 2-3 size-1, 4 bold, 5 italic, 6 underline
_fonts: .addr   _font_sans, _font_serif, _font_mono, _font_sans
_fbms:  .addr   _fbm_sans, _fbm_serif, _fbm_mono, _fbm_sans
_mx:    .word   280
_my:    .byte   96
_mbtn:  .byte   0
_mslot: .byte   0
_pshape: .byte  0               ; 0 arrow, 1 I-beam, 2 watch
_ptr_on: .byte  0
_lbx:   .word   0
_lm_n:  .byte   0
_lm_tx: .word   0
_lm_s0: .byte   $FF
_lm_s1: .byte   $FF
_lm_x0: .word   0
_lm_x1: .word   0
_lm_i:  .byte   0
_lm_nf: .byte   0
_lm_asc: .byte  0
_lm_desc: .byte 0
_lm_cr: .byte   0
_lm_pa: .byte   0               ; paragraph attributes (in/out)
_lm_np: .byte   0
_lm_ns: .byte   0
_lm_jn: .byte   0
_lm_jx: .byte   0
_lm_jr: .byte   0
_lm_w:  .word   0
_wrapx: .word   524
_tabbase: .word 12
_tabs:  .res    16, 0           ; up to 8 absolute tab stops, 0 ends
cgx:    .word   $FFFF          ; pen x whose column is cached

        .code

; ------------------------------------------------------------------------
_gfx_on:
        sta     $C001           ; 80STORE
        sta     $C00D           ; 80COL
        sta     PAGE2OFF
        sta     $C057           ; HIRES
        sta     $C052           ; full screen
        sta     $C05E           ; AN3 off -> double hi-res
        sta     $C050           ; graphics
        rts

_gfx_off:
        sta     $C051           ; text
        sta     $C056           ; lores
        sta     $C05F           ; AN3 on
        sta     PAGE2OFF
        rts

; ------------------------------------------------------------------------
; div7: A/X = x (lo/hi) -> A = x / 7, X = x mod 7
_div7:
        sta     zt
        stx     zt2
        ldx     #0
@big:   lda     zt2
        bne     @sub
        lda     zt
        cmp     #56
        bcc     @small
@sub:   sec
        lda     zt
        sbc     #56
        sta     zt
        lda     zt2
        sbc     #0
        sta     zt2
        txa
        clc
        adc     #8
        tax
        bra     @big
@small: lda     zt
@s7:    cmp     #7
        bcc     @done
        sbc     #7
        inx
        bra     @s7
@done:  sta     zt
        txa
        ldx     zt
        rts

; ------------------------------------------------------------------------
setrow:                         ; zrow = LB row A
        tax
        lda     lbrlo,x
        sta     zrow
        lda     lbrhi,x
        sta     zrow+1
        rts

; void __fastcall__ lb_clear (unsigned char nrows)
_lb_clear:
        sta     zcnt
        lda     #0
        jsr     setrow
@r:     lda     #0
        ldy     #79
@c:     sta     (zrow),y
        dey
        bpl     @c
        clc
        lda     zrow
        adc     #80
        sta     zrow
        bcc     :+
        inc     zrow+1
:       dec     zcnt
        bne     @r
        rts

; void __fastcall__ lb_fill (unsigned x0, unsigned x1, unsigned char r0,
;                            unsigned char r1, unsigned char mode)
; Rows r0..r1, pixels x0..x1-1. Modes: 0 clear, 1 set, 2 xor, 3 50% gray.
_lb_fill:
        sta     fmode
        jsr     popa
        sta     fr1
        jsr     popa
        sta     fr0
        jsr     popax
        sta     fx1
        stx     fx1+1
        jsr     popax
        sta     fx0
        stx     fx0+1
fill_go:
        lda     fx0
        ldx     fx0+1
        jsr     _div7
        sta     fc0
        lda     lmask,x
        sta     fm0
        lda     fx1
        ldx     fx1+1
        jsr     _div7
        sta     fc1
        lda     rmask,x
        sta     fm1
        lda     fr0
        sta     zrowi
@row:   lda     zrowi
        cmp     #LBROWS
        bcs     @done
        jsr     setrow
        ldy     fc0
@col:   cpy     fc1
        beq     @last
        bcs     @next
        lda     #$7F
        cpy     fc0
        bne     :+
        lda     fm0
:       jsr     applym
        iny
        bra     @col
@last:  lda     fm1
        cpy     fc0
        bne     :+
        and     fm0
:       jsr     applym
@next:  inc     zrowi
        lda     zrowi
        cmp     fr1
        beq     @row
        bcc     @row
@done:  rts

; apply mask A at column Y of the current row (fmode)
applym:
        sta     zt
        beq     @rts
        lda     fmode
        beq     @clr
        cmp     #2
        beq     @xor
        cmp     #3
        beq     @gray
        cmp     #4
        beq     @dim
        lda     zt              ; set
        ora     (zrow),y
        sta     (zrow),y
@rts:   rts
@clr:   lda     zt
        eor     #$7F
        and     (zrow),y
        sta     (zrow),y
        rts
@xor:   lda     zt
        eor     (zrow),y
        sta     (zrow),y
        rts
@dim:   tya                     ; keep only pattern pixels (dimmed text)
        clc
        adc     zrowi
        and     #1
        tax
        lda     zt
        eor     #$7F
        ora     gray,x
        and     (zrow),y
        sta     (zrow),y
        rts
@gray:  tya                     ; pattern by (column + row) parity
        clc
        adc     zrowi
        and     #1
        tax
        lda     gray,x
        and     zt
        sta     zt2
        lda     zt
        eor     #$7F
        and     (zrow),y
        ora     zt2
        sta     (zrow),y
        rts

; ------------------------------------------------------------------------
; fonts and glyphs

selfont:                        ; from _gfmt: zfp, zscale
        lda     _gfmt
        and     #3
        asl     a
        tax
        lda     _fonts,x
        sta     zfp
        lda     _fonts+1,x
        sta     zfp+1
        lda     _gfmt           ; effective style, baseline shift
        sta     geff
        stz     zvoff
        stz     fasc
        stz     fdesc
        lsr     a
        lsr     a
        and     #3
        cmp     #3
        bne     @norm
        lda     _gfmt           ; size 3: superscript (bold bit clear)
        and     #$EF            ;   or subscript (bold bit set), at 9 point
        sta     geff
        lda     _gfmt
        and     #$10
        beq     @sup
        lda     #2              ; subscript: 2 rows down
        sta     zvoff
        sta     fdesc
        bra     @small
@sup:   lda     #$FD            ; superscript: 3 rows up
        sta     zvoff
        lda     #3
        sta     fasc
@small: lda     #0
@norm:  inc     a
        sta     zscale
        asl     a               ; descent = 2 * scale (+ shift)
        clc
        adc     fdesc
        sta     fdesc
        lda     zscale          ; ascent = 8 * scale (+ shift)
        asl     a
        asl     a
        asl     a
        clc
        adc     fasc
        sta     fasc
        rts

gindex:                         ; char A -> glyph index in Y
        cmp     #$09
        bne     :+
        lda     #$20
:       sec
        sbc     #$20
        cmp     #100
        bcc     :+
        lda     #$1F            ; '?'
:       tay
        rts

; unsigned char __fastcall__ char_width (unsigned char ch)  (with _gfmt)
_char_width:
        pha
        jsr     selfont
        pla
cwidth:
        jsr     gindex
        iny
        iny
        lda     (zfp),y
        and     #$3F
        clc
        adc     #1
        sta     zt
        lda     #0
        ldx     zscale
:       clc
        adc     zt
        dex
        bne     :-
        sta     zt
        lda     geff
        and     #$10
        beq     :+
        inc     zt
:       lda     zt
        ldx     #0
        rts

; unsigned char __fastcall__ draw_char (unsigned char ch)
_draw_char:
        sta     dch
        jsr     selfont
        lda     dch
        jsr     cwidth
        sta     dadv
        lda     dch
        cmp     #$21
        bcs     :+
        jmp     @underl         ; blank
:       jsr     gindex
        sty     zt
        iny
        iny
        lda     (zfp),y         ; width | offset hi << 6
        tax
        and     #$3F
        sta     zw
        txa
        lsr     a
        lsr     a
        lsr     a
        lsr     a
        lsr     a
        lsr     a
        sta     zgp+1
        lda     zt              ; offset lo at +102+i
        clc
        adc     #102
        tay
        lda     (zfp),y
        sta     zgp
        lda     _gfmt           ; zgp = bitmap base + offset (aux memory)
        and     #3
        asl     a
        tax
        lda     zgp
        clc
        adc     _fbms,x
        sta     zgp
        lda     zgp+1
        adc     _fbms+1,x
        sta     zgp+1
        lda     zfp             ; top << 4 | rows at +202+i (index may exceed 255)
        clc
        adc     #202
        sta     zsave
        lda     zfp+1
        adc     #0
        sta     zsave+1
        ldy     zt
        lda     (zsave),y
        tax
        lsr     a
        lsr     a
        lsr     a
        lsr     a
        sta     zgr             ; first inked row
        txa
        and     #$0F
        bne     :+
        jmp     @underl
:       sta     gcnt
        ldx     zw              ; fetch the glyph rows from aux memory
        cpx     #9
        bcc     :+
        asl     a
:       tay
        jsr     auxglyph
        lda     #<gbuf
        sta     zgp
        lda     #>gbuf
        sta     zgp+1
        ; first row = gbase + 1 - 8*scale + top*scale (may wrap: skipped)
        lda     zscale
        asl     a
        asl     a
        asl     a
        sta     zt
        lda     _gbase
        clc
        adc     zvoff
        clc
        adc     #1
        sec
        sbc     zt
        ldx     zscale
:       clc
        adc     zgr
        dex
        bne     :-
        sta     zrowi
        ; base column / shift of the pen (cached from the previous glyph)
        lda     _gx
        cmp     cgx
        bne     @dodiv
        lda     _gx+1
        cmp     cgx+1
        beq     @cached
@dodiv: lda     _gx
        ldx     _gx+1
        jsr     _div7
        sta     ccol
        stx     cshift
        lda     _gx
        sta     cgx
        lda     _gx+1
        sta     cgx+1
@cached:
        lda     ccol
        sta     bcol
        lda     cshift
        sta     bshift
        lda     zscale
        cmp     #1
        beq     @fast
        jmp     @slow

; ---- fast path: normal size, rows of at most 13 bits after bold
@fast:  ldy     #0
        lda     (zgp),y
        sta     zd0
        stz     zd1
        stz     zd2
        inc     zgp
        bne     :+
        inc     zgp+1
:       lda     zw
        cmp     #9
        bcc     :+
        lda     (zgp),y
        sta     zd1
        inc     zgp
        bne     :+
        inc     zgp+1
:       lda     zd0
        ora     zd1
        bne     :+
        jmp     @fnext
:       lda     zrowi
        cmp     #LBROWS
        bcc     :+
        jmp     @fnext
:       jsr     setrow
        lda     geff
        and     #$30
        beq     @fplain
        and     #$10
        beq     :+
        lda     zd0             ; bold
        asl     a
        sta     zt
        lda     zd1
        rol     a
        sta     zt2
        lda     #0
        rol     a
        sta     zd2
        lda     zd0
        ora     zt
        sta     zd0
        lda     zd1
        ora     zt2
        sta     zd1
:       lda     geff
        and     #$20
        beq     @fplain
        lda     #9              ; italic: extra (9-r)/2 pixels
        sec
        sbc     zgr
        lsr     a
        clc
        adc     bshift
        bra     @fsh
@fplain:
        lda     bshift
@fsh:   ldx     bcol
:       cmp     #7
        bcc     :+
        sbc     #7
        inx
        bra     :-
:       stx     zcol
        tay
        lda     zd1
        ora     zd2
        bne     @wide
        ; narrow: 8-bit row -> 2 pieces
        lda     zd0
        cpy     #0
        beq     @n0
:       asl     a
        rol     zd1
        dey
        bne     :-
@n0:    ldy     zcol
        cpy     #80
        bcs     @fnext
        tax
        and     #$7F
        beq     :+
        ora     (zrow),y
        sta     (zrow),y
:       iny
        cpy     #80
        bcs     @fnext
        txa
        asl     a
        lda     zd1
        rol     a
        beq     @fnext
        ora     (zrow),y
        sta     (zrow),y
        bra     @fnext
@wide:  tya
        beq     @fput
:       asl     zd0
        rol     zd1
        rol     zd2
        dey
        bne     :-
@fput:  ldy     zcol
        cpy     #80
        bcs     @fnext
        lda     zd0             ; piece 0: bits 0-6
        and     #$7F
        beq     :+
        ora     (zrow),y
        sta     (zrow),y
:       iny
        cpy     #80
        bcs     @fnext
        lda     zd0             ; piece 1: bits 7-13
        asl     a
        lda     zd1
        rol     a
        and     #$7F
        beq     :+
        ora     (zrow),y
        sta     (zrow),y
:       iny
        cpy     #80
        bcs     @fnext
        lda     zd1             ; piece 2: bits 14-20
        asl     a
        sta     zt
        lda     zd2
        rol     a
        asl     zt
        rol     a
        and     #$7F
        beq     @fnext
        ora     (zrow),y
        sta     (zrow),y
@fnext: inc     zrowi
        inc     zgr
        dec     gcnt
        beq     :+
        jmp     @fast
:       jmp     @underl

; ---- general path: 2x / 3x sizes
@slow:
@grow:  ldy     #0
        lda     (zgp),y
        sta     zd0
        stz     zd1
        stz     zd2
        inc     zgp
        bne     :+
        inc     zgp+1
:       lda     zw
        cmp     #9
        bcc     @have
        lda     (zgp),y
        sta     zd1
        inc     zgp
        bne     @have
        inc     zgp+1
@have:  lda     zd0
        ora     zd1
        bne     :+
        jmp     @skiprow
:       lda     geff            ; bold: d |= d << 1
        and     #$10
        beq     @nob
        lda     zd0
        asl     a
        sta     zt
        lda     zd1
        rol     a
        sta     zt2
        lda     #0
        rol     a
        sta     zd2
        lda     zd0
        ora     zt
        sta     zd0
        lda     zd1
        ora     zt2
        sta     zd1
@nob:   jsr     hscale          ; -> zv, zn
        lda     bshift
        sta     zt2
        lda     geff
        and     #$20
        beq     @noit
        lda     #9
        sec
        sbc     zgr
        lsr     a
        sta     zt
        lda     #0
        ldx     zscale
:       clc
        adc     zt
        dex
        bne     :-
        clc
        adc     zt2
        sta     zt2
@noit:  lda     zt2
        ldx     bcol
:       cmp     #7
        bcc     :+
        sbc     #7
        inx
        bra     :-
:       stx     zcol
        sta     zs
        ldx     zn
        stz     zv,x
        inc     zn
        ldy     zs
        beq     @shd
@sh:    asl     zv
        rol     zv+1
        rol     zv+2
        rol     zv+3
        rol     zv+4
        rol     zv+5
        dey
        bne     @sh
@shd:   ldx     #5
:       lda     zv,x
        sta     zvs,x
        dex
        bpl     :-
        lda     zscale
        sta     zrep
@rep:   lda     zrowi
        cmp     #LBROWS
        bcs     @norow
        jsr     setrow
        ldx     #5
:       lda     zvs,x
        sta     zv,x
        dex
        bpl     :-
        jsr     orpieces
@norow: inc     zrowi
        dec     zrep
        bne     @rep
        bra     @nextrow
@skiprow:
        lda     zrowi
        clc
        adc     zscale
        sta     zrowi
@nextrow:
        inc     zgr
        dec     gcnt
        beq     @underl
        jmp     @grow
@underl:
        lda     geff
        and     #$40
        beq     @adv
        lda     _gbase          ; underline row = baseline + scale
        clc
        adc     zvoff
        clc
        adc     zscale
        sta     fr0
        sta     fr1
        lda     _gx
        sta     fx0
        clc
        adc     dadv
        sta     fx1
        lda     _gx+1
        sta     fx0+1
        adc     #0
        sta     fx1+1
        lda     #1
        sta     fmode
        jsr     fill_go
@adv:   lda     _gx
        cmp     cgx             ; cache valid for the current gx?
        bne     @nocache
        lda     _gx+1
        cmp     cgx+1
        bne     @nocache
        lda     cshift          ; advance cached column/shift by dadv
        clc
        adc     dadv
        ldx     ccol
:       cmp     #7
        bcc     :+
        sbc     #7
        inx
        bra     :-
:       sta     cshift
        stx     ccol
        lda     _gx
        clc
        adc     dadv
        sta     _gx
        sta     cgx
        lda     _gx+1
        adc     #0
        sta     _gx+1
        sta     cgx+1
        lda     dadv
        ldx     #0
        rts
@nocache:
        lda     _gx
        clc
        adc     dadv
        sta     _gx
        bcc     :+
        inc     _gx+1
:       lda     #$FF            ; invalidate
        sta     cgx+1
        lda     dadv
        ldx     #0
        rts

; horizontal scaling of zd0..zd2 into zv (zn bytes)
hscale:
        stz     zv+3
        stz     zv+4
        stz     zv+5
        lda     zscale
        cmp     #1
        bne     @x2
        lda     zd0
        sta     zv
        lda     zd1
        sta     zv+1
        lda     zd2
        sta     zv+2
        lda     #3
        sta     zn
        rts
@x2:    cmp     #2
        bne     @x3
        ldx     #0
        lda     zd0
        jsr     exp2
        lda     zd1
        jsr     exp2
        lda     zd2
        and     #$0F
        tay
        lda     ex2tab,y
        sta     zv,x
        inx
        stx     zn
        rts
@x3:    stz     zv
        stz     zv+1
        stz     zv+2
        lda     zd0             ; t = d << 3 so bit 12 sits in bit 15
        sta     zt
        lda     zd1
        sta     zt2
        ldx     #3
:       asl     zt
        rol     zt2
        dex
        bne     :-
        lda     #13
        sta     zcnt
@b:     asl     zt
        rol     zt2             ; C = next bit (MSB first)
        lda     #0
        rol     a
        sta     zrep            ; temp bit
        ldx     #3
@r:     lda     zrep
        lsr     a
        rol     zv
        rol     zv+1
        rol     zv+2
        rol     zv+3
        rol     zv+4
        rol     zv+5
        dex
        bne     @r
        dec     zcnt
        bne     @b
        lda     #5
        sta     zn
        rts

exp2:   pha
        and     #$0F
        tay
        lda     ex2tab,y
        sta     zv,x
        inx
        pla
        lsr     a
        lsr     a
        lsr     a
        lsr     a
        tay
        lda     ex2tab,y
        sta     zv,x
        inx
        rts

; OR value zv (8 bits/byte, LSB-left) into current row from column zcol,
; as 7-bit pieces.
orpieces:
        ldy     zcol
@p:     cpy     #80
        bcs     @done
        lda     zv
        and     #$7F
        beq     :+
        ora     (zrow),y
        sta     (zrow),y
:       iny
        asl     zv              ; value >>= 7
        rol     zv+1
        rol     zv+2
        rol     zv+3
        rol     zv+4
        rol     zv+5
        lda     zv+1
        sta     zv
        lda     zv+2
        sta     zv+1
        lda     zv+3
        sta     zv+2
        lda     zv+4
        sta     zv+3
        lda     zv+5
        sta     zv+4
        stz     zv+5
        ora     zv
        ora     zv+1
        ora     zv+2
        ora     zv+3
        ora     zv+4
        bne     @p
@done:  rts


; ------------------------------------------------------------------------
; Line engine: walks TB[0.._lm_n-1] (text with format markers) starting at
; pen _gx with format _gfmt and paragraph attributes _lm_pa.
;   mode 0  break   - find where the line wraps (_wrapx), CR ends a line
;   mode 1  measure - advance _gx over all characters
;   mode 2  hit     - stop at the character under x = _lm_tx
;   mode 3  draw    - draw everything; record _gx at indexes _lm_s0/_lm_s1
; Markers: $80|fmt sets the character format; a marker whose font bits are
; 3 is a paragraph marker ($83|align<<2|spacing<<4) and sets _lm_pa.
; TAB advances to the next stop in _tabs (absolute x, 0-terminated), then
; every 40 pixels from _tabbase. In modes 1-3 the first _lm_jn spaces get
; _lm_jx extra pixels (+1 for the first _lm_jr of them): full justification.
; Results: _lm_i (index), _lm_nf (format there), _lm_np (paragraph attributes
; for the next line), _lm_asc/_lm_desc, _lm_w (right edge of the text,
; without a trailing space at a wrap) and _lm_ns (spaces before that point,
; 0 unless the line wrapped) in mode 0, _lm_cr (1 if a CR ended the line,
; 2 if the text ran out).
TB      = $1E00

_line_engine:
        sta     lmmode
        stz     _lm_i
        stz     _lm_cr
        stz     lmns
        stz     lmwrap
        lda     #$FF
        sta     lmbp
        jsr     selfont
        lda     fdesc           ; ascent/descent of the starting format
        sta     _lm_desc
        lda     fasc
        sta     _lm_asc
@loop:  lda     _lm_i
        cmp     _lm_n
        bne     :+
        jmp     @end
:       lda     lmmode
        cmp     #3
        bne     @nocap
        lda     _lm_i
        cmp     _lm_s0
        bne     :+
        lda     _gx
        sta     _lm_x0
        lda     _gx+1
        sta     _lm_x0+1
        lda     _lm_i
:       cmp     _lm_s1
        bne     @nocap
        lda     _gx
        sta     _lm_x1
        lda     _gx+1
        sta     _lm_x1+1
@nocap: ldy     _lm_i
        lda     TB,y
        bpl     @char
        and     #$7F            ; marker
        tax
        and     #3
        cmp     #3
        bne     :+
        stx     _lm_pa          ; paragraph attributes
        inc     _lm_i
        bra     @loop
:       stx     _gfmt           ; character format
        jsr     selfont
        inc     _lm_i
        bra     @loop
@char:  cmp     #$0C            ; a page break ends the line like a CR
        beq     :+
        cmp     #$0D
        bne     @notcr
:       lda     lmmode
        beq     @crbrk
        cmp     #2
        bne     :+
        jmp     @done           ; hit: a CR ends the line
:       inc     _lm_i
        bra     @loop
@crbrk: inc     _lm_i
        lda     #1
        sta     _lm_cr
        jmp     @done
@notcr: sta     lmch
        cmp     #$09
        bne     @nottab
        jsr     tabw            ; A = distance to the next tab stop
        bra     @gotw
@nottab: lda    lmmode
        cmp     #3
        bne     :+
        lda     lmch            ; draw
        jsr     _draw_char
        jsr     justx
        beq     @dnext
        clc                     ; extra space for justification
        adc     _gx
        sta     _gx
        bcc     @dnext
        inc     _gx+1
@dnext: inc     _lm_i
        jmp     @loop
:       lda     lmch
        jsr     cwidth          ; A = width (uses zfp/zscale)
        sta     lmw
        lda     lmmode
        beq     @gotw2
        jsr     justx
        clc
        adc     lmw
@gotw:  sta     lmw
@gotw2: lda     lmmode
        beq     @brk
        cmp     #2
        beq     @hit
        ; measure (and draw for a TAB)
@adv:   lda     _gx
        clc
        adc     lmw
        sta     _gx
        bcc     :+
        inc     _gx+1
:       inc     _lm_i
        jmp     @loop
@hit:   lda     lmw             ; stop if tx < gx + w/2
        lsr     a
        clc
        adc     _gx
        sta     zt
        lda     _gx+1
        adc     #0
        sta     zt2
        lda     _lm_tx+1
        cmp     zt2
        bcc     @hdone
        bne     @adv
        lda     _lm_tx
        cmp     zt
        bcc     @hdone
        bra     @adv
@hdone: jmp     @done
@brk:   ; would gx + w pass _wrapx?  (spaces may hang over the edge)
        lda     _gx
        clc
        adc     lmw
        sta     zt
        lda     _gx+1
        adc     #0
        sta     zt2
        lda     lmch
        cmp     #' '
        beq     @fits
        lda     zt2
        cmp     _wrapx+1
        bcc     @fits
        bne     @wrap
        lda     _wrapx
        cmp     zt
        bcs     @fits
@wrap:  lda     lmbp
        cmp     #$FF
        beq     :+
        sta     _lm_i           ; break after the last space
        lda     lmbf
        sta     _gfmt
        lda     #1
        sta     lmwrap
        bra     @done
:       lda     _lm_i
        bne     @done
        inc     _lm_i           ; at least one character per line
        bra     @done
@fits:  lda     lmch
        cmp     #' '
        bne     :+
        lda     _gx             ; a break point: remember the width so far
        sta     lmbx
        lda     _gx+1
        sta     lmbx+1
        lda     lmns
        sta     lmbn
        inc     lmns
:       lda     zt
        sta     _gx
        lda     zt2
        sta     _gx+1
        lda     fdesc           ; grow ascent/descent
        cmp     _lm_desc
        bcc     :+
        sta     _lm_desc
:       lda     fasc
        cmp     _lm_asc
        bcc     :+
        sta     _lm_asc
:       inc     _lm_i
        lda     lmch
        cmp     #' '
        bne     :+
        lda     _lm_i
        sta     lmbp
        lda     _gfmt
        sta     lmbf
:       jmp     @loop
@end:   lda     lmmode          ; ran out of text
        bne     @done
        lda     #2
        sta     _lm_cr          ; 2 = end of buffer
@done:  lda     lmmode
        bne     @nomode0
        lda     lmwrap          ; width and spaces of the line
        beq     :+
        lda     lmbx
        sta     _lm_w
        lda     lmbx+1
        sta     _lm_w+1
        lda     lmbn
        bra     @ns
:       lda     _gx
        sta     _lm_w
        lda     _gx+1
        sta     _lm_w+1
        lda     #0
@ns:    sta     _lm_ns
@nomode0:
        lda     _lm_pa          ; next line's paragraph attributes
        ldx     _lm_cr
        cpx     #1
        bne     :+
        lda     #0
:       sta     _lm_np
        lda     lmmode
        cmp     #3
        bne     :+
        lda     _lm_i           ; capture at the end position too
        cmp     _lm_s1
        bne     :+
        lda     _gx
        sta     _lm_x1
        lda     _gx+1
        sta     _lm_x1+1
:       lda     _gfmt
        sta     _lm_nf
        lda     _lm_i
        ldx     #0
        rts

; A = extra justification pixels for this character (Z set if none)
justx:  lda     lmch
        cmp     #' '
        bne     @no
        lda     _lm_jn
        beq     @no
        dec     _lm_jn
        lda     _lm_jr
        beq     :+
        dec     _lm_jr
        lda     _lm_jx
        inc     a
        rts
:       lda     _lm_jx
        rts
@no:    lda     #0
        rts

; A = pixels from _gx to the next tab stop (at most 255)
tabw:   ldx     #0
@t:     lda     _tabs+1,x       ; stop = _tabs[x/2]; 0 ends the list
        ora     _tabs,x
        beq     @dflt
        lda     _gx
        cmp     _tabs,x
        lda     _gx+1
        sbc     _tabs+1,x
        bcc     @use            ; gx < stop
        inx
        inx
        cpx     #16
        bcc     @t
@dflt:  lda     _tabbase        ; default stops every 40 pixels
        sta     zt
        lda     _tabbase+1
        sta     zt2
@d:     lda     _gx
        cmp     zt
        lda     _gx+1
        sbc     zt2
        bcc     @dz             ; gx < stop
        lda     zt
        clc
        adc     #40
        sta     zt
        bcc     @d
        inc     zt2
        bra     @d
@use:   lda     _tabs,x
        sta     zt
        lda     _tabs+1,x
        sta     zt2
@dz:    lda     zt
        sec
        sbc     _gx
        tax
        lda     zt2
        sbc     _gx+1
        beq     :+
        ldx     #255
:       txa
        bne     :+
        lda     #1
:       rts

; ------------------------------------------------------------------------
; void __fastcall__ lb_cmds (const unsigned char *list)
; Records of 7 bytes: mode, x0 (word), x1 (word), r0, r1; mode $FF ends.
; x values are relative to _lbx.
_lb_cmds:
        sta     zsave
        stx     zsave+1
@rec:   ldy     #0
        lda     (zsave),y
        cmp     #$FF
        beq     @end
        sta     fmode
        iny
        lda     (zsave),y
        clc
        adc     _lbx
        sta     fx0
        iny
        lda     (zsave),y
        adc     _lbx+1
        sta     fx0+1
        iny
        lda     (zsave),y
        clc
        adc     _lbx
        sta     fx1
        iny
        lda     (zsave),y
        adc     _lbx+1
        sta     fx1+1
        iny
        lda     (zsave),y
        sta     fr0
        iny
        lda     (zsave),y
        sta     fr1
        lda     zsave
        pha
        lda     zsave+1
        pha
        jsr     fill_go
        pla
        sta     zsave+1
        pla
        clc
        adc     #7
        sta     zsave
        bcc     @rec
        inc     zsave+1
        bra     @rec
@end:   rts

; ------------------------------------------------------------------------
; void __fastcall__ present (unsigned char y, unsigned char nrows,
;                            unsigned char c0, unsigned char c1)
_present:
        sta     pc1
        jsr     popa
        sta     pc0
        jsr     popa
        sta     pn
        jsr     popa
        sta     py
        lda     pn
        bne     :+
        rts
:       ; byte offsets: even cols c0e..c1e -> aux offsets, odd -> main
        lda     pc0             ; aux: columns 2k, k from ceil(c0/2) .. floor(c1/2)
        clc
        adc     #1
        lsr     a
        sta     pa0
        lda     pc1
        lsr     a
        sta     pa1
        lda     pc0             ; main: columns 2k+1, k from floor(c0/2) .. floor((c1-1)/2)
        lsr     a
        sta     pm0_
        lda     pc1
        sec
        sbc     #1
        lsr     a
        sta     pm1_
        stz     zrowi
@row:   ldx     zrowi
        lda     lbrlo,x
        sta     @ra+1
        sta     @rm+1
        lda     lbrhi,x
        sta     @ra+2
        sta     @rm+2
        ldx     py
        lda     ylo,x
        sta     zscr
        lda     yhi,x
        sta     zscr+1
        sta     PAGE2ON         ; aux: screen[k] <- LB[2k]
        ldy     pa0
        cpy     pa1
        beq     :+
        bcs     @auxd
:       tya
        asl     a
        tax
:
@ra:    lda     $FFFF,x
        eor     #$7F
        sta     (zscr),y
        inx
        inx
        iny
        cpy     pa1
        bcc     :-
        beq     :-
@auxd:  sta     PAGE2OFF        ; main: screen[k] <- LB[2k+1]
        ldy     pm0_
        cpy     pm1_
        beq     :+
        bcs     @maind
:       tya
        asl     a
        tax
        inx
:
@rm:    lda     $FFFF,x
        eor     #$7F
        sta     (zscr),y
        inx
        inx
        iny
        cpy     pm1_
        bcc     :-
        beq     :-
@maind: inc     py
        inc     zrowi
        lda     zrowi
        cmp     pn
        beq     :+
        jmp     @row
:       rts

; ------------------------------------------------------------------------
; void __fastcall__ scr_copy (unsigned char src_y, unsigned char dst_y,
;                             unsigned char n, unsigned char c0, unsigned char c1)
_scr_copy:
        sta     pc1
        jsr     popa
        sta     pc0
        jsr     popa
        sta     pn
        jsr     popa
        sta     py
        jsr     popa
        sta     psy
        lda     pn
        beq     @rts
        lda     pc0
        lsr     a
        sta     zb0
        lda     pc1
        lsr     a
        sta     zb1
        lda     py
        cmp     psy
        bcc     @up
        lda     psy
        clc
        adc     pn
        sta     psy
        lda     py
        clc
        adc     pn
        sta     py
@dn:    dec     psy
        dec     py
        jsr     copyrow
        dec     pn
        bne     @dn
@rts:   rts
@up:    jsr     copyrow
        inc     psy
        inc     py
        dec     pn
        bne     @up
        rts

copyrow:
        ldx     psy
        lda     ylo,x
        sta     zsave
        lda     yhi,x
        sta     zsave+1
        ldx     py
        lda     ylo,x
        sta     zscr
        lda     yhi,x
        sta     zscr+1
        sta     PAGE2ON
        ldy     zb0
:       lda     (zsave),y
        sta     (zscr),y
        iny
        cpy     zb1
        bcc     :-
        beq     :-
        sta     PAGE2OFF
        ldy     zb0
:       lda     (zsave),y
        sta     (zscr),y
        iny
        cpy     zb1
        bcc     :-
        beq     :-
        rts

; ------------------------------------------------------------------------
; Save-under: a screen rectangle <-> aux $0800 buffer.
; void __fastcall__ scr_save (unsigned char c0, unsigned char c1,
;                             unsigned char y0, unsigned char y1)
; void scr_restore (void)   (restores the last saved rectangle)
_scr_save:
        sta     sy1
        jsr     popa
        sta     sy0
        jsr     popa
        lsr     a
        sta     sb1
        jsr     popa
        lsr     a
        sta     sb0
        lda     #1
        bra     saverest
_scr_restore:
        lda     #0
saverest:
        sta     zcnt            ; 1 = save, 0 = restore
        lda     sb0
        sta     zb0
        lda     sb1
        sta     zb1
        lda     #<SAVEBUF
        sta     zsave
        lda     #>SAVEBUF
        sta     zsave+1
        lda     sy0
        sta     zrowi
@row:   lda     zsave+1         ; never past the buffer ($1FFF): a box too big
        cmp     #$1F            ; to save is only partly restored
        bcc     :+
        lda     zsave
        cmp     #$AE
        bcs     @full
:       ldx     zrowi
        lda     ylo,x
        sta     zscr
        lda     yhi,x
        sta     zscr+1
        sta     PAGE2ON
        jsr     sr_bank         ; aux bank bytes
        sta     PAGE2OFF
        jsr     sr_bank         ; main bank bytes
        inc     zrowi
        lda     zrowi
        cmp     sy1
        beq     @row
        bcc     @row
@full:  rts


; ------------------------------------------------------------------------
; void __fastcall__ caret_xor (unsigned x, unsigned char y0, unsigned char y1)
; Inverts a one-pixel vertical line on the screen (rows y0..y1).
_caret_xor:
        sta     sy1
        jsr     popa
        sta     sy0
        jsr     popax
        jsr     _div7
        sta     zt              ; column
        lda     bitv,x
        sta     zt2             ; bit mask
        lda     zt
        lsr     a
        sta     zb0             ; byte offset
        lda     zt
        and     #1
        bne     :+
        sta     PAGE2ON
        bra     :++
:       sta     PAGE2OFF
:       ldx     sy0
@lp:    lda     ylo,x
        sta     zscr
        lda     yhi,x
        sta     zscr+1
        ldy     zb0
        lda     (zscr),y
        eor     zt2
        sta     (zscr),y
        cpx     sy1
        beq     @done
        inx
        bra     @lp
@done:  sta     PAGE2OFF
        rts

; ------------------------------------------------------------------------
; Mouse (firmware, polled)

; unsigned char mouse_init (void): slot 1-7, or 0 if no mouse (startup only)
        .segment "STARTCODE"
_mouse_init:
        ldx     #7
@scan:  txa
        ora     #$C0
        sta     zt2
        stz     zt
        ldy     #$0C
        lda     (zt),y
        cmp     #$20
        bne     @next
        ldy     #$FB
        lda     (zt),y
        cmp     #$D6
        beq     @found
@next:  dex
        bne     @scan
        lda     #0
        tax
        rts
@found: stx     _mslot
        txa
        asl     a
        asl     a
        asl     a
        asl     a
        sta     mn0
        lda     zt2
        sta     mcn
        lda     #0
        ldy     #$19            ; INITMOUSE
        jsr     mcall
        lda     #0              ; clamp X 0..559
        sta     $0478
        sta     $0578
        lda     #<559
        sta     $04F8
        lda     #>559
        sta     $05F8
        lda     #0
        ldy     #$17
        jsr     mcall
        lda     #0              ; clamp Y 0..191
        sta     $0478
        sta     $0578
        sta     $05F8
        lda     #191
        sta     $04F8
        lda     #1
        ldy     #$17
        jsr     mcall
        lda     #1              ; SETMOUSE: on, no interrupts
        ldy     #$12
        jsr     mcall
        ldx     _mslot          ; POSMOUSE 280,96
        lda     #<280
        sta     $0478,x
        lda     #>280
        sta     $0578,x
        lda     #96
        sta     $04F8,x
        lda     #0
        sta     $05F8,x
        ldy     #$16
        jsr     mcall
        lda     _mslot
        ldx     #0
        rts

        .code
mcall:  sta     zt              ; A = parameter, Y = entry table offset
        lda     mcn
        sta     @jmp+2
        sta     zsave+1
        stz     zsave
        lda     (zsave),y
        sta     @jmp+1
        lda     zt
        ldx     mcn
        ldy     mn0
        php
        sei
        sta     PAGE2OFF
@jmp:   jsr     $FFFF
        plp
        rts

; void mouse_poll (void)
_mouse_poll:
        lda     _mslot
        beq     @rts
        ldy     #$14            ; READMOUSE
        jsr     mcall
        ldx     _mslot
        lda     $0478,x
        sta     _mx
        lda     $0578,x
        sta     _mx+1
        lda     $04F8,x
        sta     _my
        lda     $0778,x
        and     #$80
        sta     _mbtn
@rts:   rts

; ------------------------------------------------------------------------
; Pointer: 16x16 shape (mask, white) drawn on screen with save-under.
_ptr_show:
        lda     _ptr_on
        beq     :+
        rts
:       lda     #1
        sta     _ptr_on
        ldx     _pshape
        lda     _mx             ; x - hotx (clamped at 0)
        sec
        sbc     hotx,x
        sta     zt
        lda     _mx+1
        sbc     #0
        bcs     :+
        lda     #0
        sta     zt
:       tax
        lda     zt
        jsr     _div7
        sta     pcol
        stx     pbit
        ldx     _pshape
        lda     _my
        sec
        sbc     hoty,x
        bcs     :+
        lda     #0
:       sta     prow
        lda     _pshape
        asl     a
        tax
        lda     shapes,x
        sta     zgp
        lda     shapes+1,x
        sta     zgp+1
        stz     zcnt            ; row 0..15
@r:     lda     zcnt
        clc
        adc     prow
        cmp     #192
        bcc     :+
        jmp     @done
:       tax
        lda     ylo,x
        sta     zscr
        lda     yhi,x
        sta     zscr+1
        lda     zcnt            ; shape row: mask lo/hi, white lo/hi
        asl     a
        asl     a
        tay
        lda     (zgp),y
        sta     zv
        iny
        lda     (zgp),y
        sta     zv+1
        iny
        lda     (zgp),y
        sta     zd0
        iny
        lda     (zgp),y
        sta     zd1
        stz     zv+2
        stz     zd2
        ldy     pbit
        beq     @ns
:       asl     zv
        rol     zv+1
        rol     zv+2
        asl     zd0
        rol     zd1
        rol     zd2
        dey
        bne     :-
@ns:    ; split into 4 seven-bit pieces: pmk[0..3], pdt[0..3]
        ldx     #0
@sp:    lda     zv
        and     #$7F
        sta     pmk,x
        lda     zd0
        and     #$7F
        sta     pdt,x
        asl     zv
        rol     zv+1
        rol     zv+2
        asl     zd0
        rol     zd1
        rol     zd2
        lda     zv+1
        sta     zv
        lda     zv+2
        sta     zv+1
        stz     zv+2
        lda     zd1
        sta     zd0
        lda     zd2
        sta     zd1
        stz     zd2
        inx
        cpx     #4
        bne     @sp
        ; draw 4 columns
        ldx     #0
@c:     txa
        clc
        adc     pcol
        cmp     #80
        bcs     @cn
        sta     zt
        lsr     a
        tay
        lda     zt
        and     #1
        bne     :+
        sta     PAGE2ON
        bra     :++
:       sta     PAGE2OFF
:       lda     (zscr),y
        pha
        ; save under at psave[row*4 + x]
        lda     zcnt
        asl     a
        asl     a
        sta     zt2
        txa
        ora     zt2
        sta     zt2
        pla
        phx
        ldx     zt2
        sta     psave,x
        plx
        ; screen bit 1 = white: new = (old & ~mask) | white
        sta     zt
        lda     pmk,x
        eor     #$7F
        and     zt
        ora     pdt,x
        sta     (zscr),y
@cn:    inx
        cpx     #4
        bne     @c
        sta     PAGE2OFF
        inc     zcnt
        lda     zcnt
        cmp     #16
        beq     @done
        jmp     @r
@done:  sta     PAGE2OFF
        rts

_ptr_hide:
        lda     _ptr_on
        bne     :+
        rts
:       stz     _ptr_on
        stz     zcnt
@r:     lda     zcnt
        clc
        adc     prow
        cmp     #192
        bcs     @done
        tax
        lda     ylo,x
        sta     zscr
        lda     yhi,x
        sta     zscr+1
        ldx     #0
@c:     txa
        clc
        adc     pcol
        cmp     #80
        bcs     @cn
        sta     zt
        lsr     a
        tay
        lda     zt
        and     #1
        bne     :+
        sta     PAGE2ON
        bra     :++
:       sta     PAGE2OFF
:       lda     zcnt
        asl     a
        asl     a
        sta     zt2
        txa
        ora     zt2
        phx
        tax
        lda     psave,x
        plx
        sta     (zscr),y
@cn:    inx
        cpx     #4
        bne     @c
        sta     PAGE2OFF
        inc     zcnt
        lda     zcnt
        cmp     #16
        bne     @r
@done:  sta     PAGE2OFF
        rts

; ------------------------------------------------------------------------
; Aux-memory transfer for save-under: runs from the language card so it
; stays visible while RAMRD selects aux memory. Copies bytes zb0..zb1 of
; the current screen row bank (selected by PAGE2) to/from (zsave)+.
        .segment "LC"
; copy Y bytes of glyph bitmap from aux (zgp) to gbuf (runs from the LC so
; that RAMRD can be switched on)
auxglyph:
        sta     $C003           ; RAMRD: read auxiliary memory
        dey
:       lda     (zgp),y
        sta     gbuf,y
        dey
        bpl     :-
        sta     $C002
        rts

sr_bank:
        php
        sei
        ldy     zb0
@lp:    lda     zcnt
        beq     @rest
        lda     (zscr),y        ; save
        sta     $C005           ; RAMWRT aux
        sty     zt
        ldy     #0
        sta     (zsave),y
        ldy     zt
        sta     $C004
        bra     @adv
@rest:  sty     zt
        ldy     #0
        sta     $C003           ; RAMRD aux
        lda     (zsave),y
        sta     $C002
        ldy     zt
        sta     (zscr),y
@adv:   inc     zsave
        bne     :+
        inc     zsave+1
:       iny
        cpy     zb1
        bcc     @lp
        beq     @lp
        plp
        rts

; ------------------------------------------------------------------------
        .rodata
lmask:  .byte   $7F,$7E,$7C,$78,$70,$60,$40     ; bits >= n
rmask:  .byte   $00,$01,$03,$07,$0F,$1F,$3F     ; bits < n
gray:   .byte   $55,$2A
bitv:   .byte   $01,$02,$04,$08,$10,$20,$40
ex2tab: .byte   $00,$03,$0C,$0F,$30,$33,$3C,$3F
        .byte   $C0,$C3,$CC,$CF,$F0,$F3,$FC,$FF
shapes: .addr   arrow, ibeam, watch
        .include "pointers.inc"

ylo:
        .repeat 192, I
        .byte   <($2000 + (I .MOD 8) * $400 + ((I / 8) .MOD 8) * $80 + (I / 64) * $28)
        .endrepeat
yhi:
        .repeat 192, I
        .byte   >($2000 + (I .MOD 8) * $400 + ((I / 8) .MOD 8) * $80 + (I / 64) * $28)
        .endrepeat
lbrlo:
        .repeat LBROWS, I
        .byte   <(LB + I * 80)
        .endrepeat
lbrhi:
        .repeat LBROWS, I
        .byte   >(LB + I * 80)
        .endrepeat

        .bss
dch:    .res    1
lmmode: .res    1
geff:   .res    1               ; format for drawing (bold off for subscript)
zvoff:  .res    1               ; baseline shift of super/subscript
fasc:   .res    1               ; ascent and descent of the current format
fdesc:  .res    1
lmns:   .res    1
lmbn:   .res    1
lmwrap: .res    1
lmbx:   .res    2
lmbp:   .res    1
lmbf:   .res    1
lmch:   .res    1
lmw:    .res    1
dadv:   .res    1
bcol:   .res    1
gcnt:   .res    1
gbuf:   .res    20              ; glyph rows fetched from aux memory
bshift: .res    1
ccol:   .res    1
cshift: .res    1
zvs:    .res    6
fmode:  .res    1
fr0:    .res    1
fr1:    .res    1
fx0:    .res    2
fx1:    .res    2
fc0:    .res    1
fc1:    .res    1
fm0:    .res    1
fm1:    .res    1
pc0:    .res    1
pc1:    .res    1
pn:     .res    1
py:     .res    1
psy:    .res    1
pa0:    .res    1
pa1:    .res    1
pm0_:   .res    1
pm1_:   .res    1
sy0:    .res    1
sy1:    .res    1
sb0:    .res    1
sb1:    .res    1
mcn:    .res    1
mn0:    .res    1
pcol:   .res    1
pbit:   .res    1
prow:   .res    1
pmk:    .res    4
pdt:    .res    4
psave:  .res    64
