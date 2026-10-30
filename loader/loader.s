;
; RetroWrite II loader (RW2.SYSTEM)
; Shows the double hi-res welcome screen (RW2.SPLASH), then loads and
; runs the editor (RW2.MAIN) at $2000.
;
        .setcpu "65C02"

MLI      = $BF00
MAINADDR = $4000
MACHID   = $BF98
DEVADR32 = $BF26
NODEV    = $BF10
PATH280  = $0280
IOBUF    = $0C00
TEMP     = $4000
KBD      = $C000
KBDSTRB  = $C010

ptr      = $06
ptr2     = $08

; ------------------------------------------------------------------
        .segment "STUB"
        ; copy body ($2100..) down to $1000 and run it
        ldx     #$0B            ; 11 pages
        ldy     #0
        lda     #$00
        sta     ptr
        sta     ptr2
        lda     #$21
        sta     ptr+1
        lda     #$10
        sta     ptr2+1
:       lda     (ptr),y
        sta     (ptr2),y
        iny
        bne     :-
        inc     ptr+1
        inc     ptr2+1
        dex
        bne     :-
        jmp     entry

; ------------------------------------------------------------------
        .segment "BODY"
entry:
        cld
        ldx     #$FF
        txs
        stz     splash_ok
        stz     $0300           ; clear handoff flag
        stz     $0301
        stz     $0302

        ; 128K?
        lda     MACHID
        and     #$30
        cmp     #$30
        bne     noshow

        ; /RAM present with files? then leave aux memory alone
        lda     DEVADR32
        cmp     NODEV
        bne     @ram
        lda     DEVADR32+1
        cmp     NODEV+1
        beq     @okram
@ram:   jsr     MLI
        .byte   $80             ; READ_BLOCK unit $B0 block 2
        .addr   rbparm
        bcs     noshow
        lda     TEMP+$25
        ora     TEMP+$26
        bne     noshow
@okram: lda     #1
        sta     splash_ok

noshow:
        jsr     getdir          ; prefix from $280

        lda     splash_ok
        beq     loadmain

        ; ---- load the splash: 8K aux half, then 8K main half
        ldx     #<splashname
        ldy     #>splashname
        jsr     openfile
        bcs     loadmain
        lda     #>TEMP
        jsr     read8k
        bcs     @fail
        sta     $C000           ; 80STORE off so RAMWRT governs $2000-$3FFF
        lda     #<TEMP
        sta     ptr
        lda     #>TEMP
        sta     ptr+1
        lda     #$00
        sta     ptr2
        lda     #$20
        sta     ptr2+1
        ldx     #$20
        ldy     #0
        php
        sei
        sta     $C005           ; RAMWRT aux
:       lda     (ptr),y
        sta     (ptr2),y
        iny
        bne     :-
        inc     ptr+1
        inc     ptr2+1
        dex
        bne     :-
        sta     $C004           ; RAMWRT main
        plp
        lda     #$20            ; main half straight to $2000
        jsr     read8k
        bcs     @fail
        jsr     closefile
        jsr     showsplash
        lda     #1
        sta     $0302           ; tell RW2.MAIN: aux used, /RAM was empty
        lda     #'R'
        sta     $0300
        lda     #'W'
        sta     $0301
        bra     loadmain
@fail:  jsr     closefile

; ---- load RW2.MAIN at $2000 and run it
loadmain:
        ldx     #<mainname
        ldy     #>mainname
        jsr     openfile
        bcs     mainerr
        lda     #<MAINADDR
        sta     rdbuf
        lda     #>MAINADDR
        sta     rdbuf+1
        lda     #<($BF00-MAINADDR)
        sta     rdlen
        lda     #>($BF00-MAINADDR)
        sta     rdlen+1
        jsr     MLI
        .byte   $CA
        .addr   rdparm
        bcc     :+
        cmp     #$4C            ; EOF is fine
        bne     mainerr
:       jsr     closefile
        jmp     MAINADDR

mainerr:
        jsr     closefile
        sta     $C001           ; 80STORE on
        sta     $C00D           ; 80 columns
        sta     $C051           ; text
        jsr     cleartext
        ldx     #<errmsg
        ldy     #>errmsg
        lda     #12
        jsr     textline
:       lda     KBD
        bpl     :-
        sta     KBDSTRB
        jsr     MLI
        .byte   $65
        .addr   quitparm
        brk

; ------------------------------------------------------------------
; build "dir/" prefix from the launch path at $280 into pathbuf
getdir:
        stz     dirlen
        lda     PATH280
        beq     @done
        cmp     #64
        bcs     @done
        tax                     ; find last '/'
@find:  lda     PATH280,x
        and     #$7F
        cmp     #'/'
        beq     @got
        dex
        bne     @find
        bra     @done           ; no slash: relative to prefix
@got:   stx     dirlen
        ldy     #1
@cp:    lda     PATH280,y
        and     #$7F
        sta     pathbuf,y
        iny
        dex
        bne     @cp
@done:  rts

; open file named by (X,Y) = length-prefixed name, prefixed by dir
; returns carry set on error
openfile:
        stx     ptr
        sty     ptr+1
        ldx     dirlen
        ldy     #0
        lda     (ptr),y         ; name length
        sta     ptr2
@cp:    iny
        lda     (ptr),y
        inx
        sta     pathbuf,x
        cpy     ptr2
        bne     @cp
        stx     pathbuf
        jsr     MLI
        .byte   $C8
        .addr   openparm
        bcs     @rts
        lda     openref
        sta     rdref
        sta     clref
@rts:   rts

closefile:
        jsr     MLI
        .byte   $CC
        .addr   clparm
        rts

; read 8192 bytes to page A
read8k:
        stz     rdbuf
        sta     rdbuf+1
        stz     rdlen
        lda     #$20
        sta     rdlen+1
        jsr     MLI
        .byte   $CA
        .addr   rdparm
        rts

; ------------------------------------------------------------------
showsplash:
        sta     $C001           ; 80STORE on
        sta     $C00D           ; 80COL
        sta     $C00F           ; ALTCHAR
        jsr     cleartext
        ldx     #<line1
        ldy     #>line1
        lda     #20
        jsr     textline
        ldx     #<line2
        ldy     #>line2
        lda     #21
        jsr     textline
        ldx     #<line3
        ldy     #>line3
        lda     #23
        jsr     textline
        sta     $C054           ; page 1
        sta     $C057           ; hires
        sta     $C053           ; mixed
        sta     $C05E           ; AN3 off -> double hi-res
        sta     $C050           ; graphics on
        sta     KBDSTRB
        ; wait about 5 seconds or a key
        ldx     #0
        ldy     #0
        lda     #6              ; ~4 seconds at 1 MHz
        sta     ptr
@w:     lda     KBD
        bmi     @key
        dey
        bne     @w
        dex
        bne     @w
        dec     ptr
        bne     @w
        bra     @off
@key:   sta     KBDSTRB
@off:   jsr     cleartext
        sta     $C051           ; text
        sta     $C052           ; full screen
        sta     $C056           ; lores
        sta     $C05F           ; AN3 on -> DHGR off
        rts

; clear 80-column text page 1 (both banks)
cleartext:
        lda     #23
@row:   pha
        ldx     #<blank
        ldy     #>blank
        jsr     textline
        pla
        dec     a
        bpl     @row
        rts

; write centered length-prefixed string (X,Y) on row A, 80 columns
; (80STORE must be on)
textline:
        stx     ptr
        sty     ptr+1
        tax
        lda     rowlo,x
        sta     ptr2
        lda     rowhi,x
        sta     ptr2+1
        ldy     #39
        lda     #$A0
@clr:   sta     $C055
        sta     (ptr2),y
        sta     $C054
        sta     (ptr2),y
        dey
        bpl     @clr
        lda     (ptr)
        beq     @done
        sta     tlen
        sec
        lda     #80
        sbc     tlen
        lsr     a
        sta     tcol
        lda     #1
        sta     tidx
@lp:    ldy     tidx
        lda     (ptr),y
        ora     #$80
        pha
        lda     tcol
        lsr     a
        tay
        bcs     @odd
        sta     $C055
        pla
        sta     (ptr2),y
        sta     $C054
        bra     @nx
@odd:   pla
        sta     (ptr2),y
@nx:    inc     tcol
        lda     tidx
        inc     tidx
        cmp     tlen
        bne     @lp
@done:  rts

tlen:   .byte   0
tcol:   .byte   0
tidx:   .byte   0

rowlo:  .byte   $00,$80,$00,$80,$00,$80,$00,$80,$28,$A8,$28,$A8
        .byte   $28,$A8,$28,$A8,$50,$D0,$50,$D0,$50,$D0,$50,$D0
rowhi:  .byte   $04,$04,$05,$05,$06,$06,$07,$07,$04,$04,$05,$05
        .byte   $06,$06,$07,$07,$04,$04,$05,$05,$06,$06,$07,$07

; ------------------------------------------------------------------
.macro PSTR s
        .byte .strlen(s), s
.endmacro
splashname: PSTR "RW2.SPLASH"
mainname:   PSTR "RW2.MAIN"
blank:      .byte 0
line1:      PSTR "R e t r o W r i t e   I I"
line2:      PSTR "A word processor for the enhanced Apple IIe"
line3:      PSTR "Press any key to begin"
errmsg:     PSTR "Can't load RW2.MAIN. Press a key to quit."

rbparm:   .byte 3, $B0
          .addr TEMP
          .word 2
openparm: .byte 3
          .addr pathbuf
          .addr IOBUF
openref:  .byte 0
rdparm:   .byte 4
rdref:    .byte 0
rdbuf:    .addr 0
rdlen:    .word 0
          .word 0
clparm:   .byte 1
clref:    .byte 0
quitparm: .byte 4, 0, 0, 0, 0, 0, 0
splash_ok: .byte 0
dirlen:   .byte 0
pathbuf:  .res 80
