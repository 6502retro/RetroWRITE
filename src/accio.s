;
; RetroWrite II - accessories overlay helpers (RW2.OVL6)
;
;   acc_rdblk(unit, block, buf)   ProDOS READ_BLOCK (for DOS 3.3 disks)
;   basic_go(path)                loads a system program at $2000 and runs it
;                                 (never returns)
;   qstub                         ProDOS quit code: when BASIC.SYSTEM quits
;                                 (BYE), it loads RW2.SYSTEM again. ProDOS
;                                 copies it from $D100 (bank 2) to $1000.
;
        .setcpu         "65C02"
        .export         _acc_rdblk, _basic_go, _qstub
        .import         popax, popa

MLI     = $BF00

        .segment        "OVL6"

; unsigned char __fastcall__ acc_rdblk (unsigned char unit, unsigned block, void *buf)
_acc_rdblk:
        sta     rbp+2           ; buffer
        stx     rbp+3
        jsr     popax
        sta     rbp+4           ; block
        stx     rbp+5
        jsr     popa
        sta     rbp+1
        jsr     MLI
        .byte   $80             ; READ_BLOCK
        .addr   rbp
        ldx     #0
        rts

rbp:    .byte   3, 0
        .word   0, 0

; void __fastcall__ basic_go (const char *path)   path: length-prefixed
; Everything else must be closed. Loads the file at $2000 (up to $A000,
; below this code) and jumps to it with the machine in its usual state.
_basic_go:
        sta     bgo+1
        stx     bgo+2
        sta     $00             ; keep the path for $0280
        stx     $01
        ldx     #$17            ; a fresh memory bitmap: a launcher such as
:       stz     $BF58,x         ; DeskTop may have left pages marked, and
        dex                     ; ProDOS then refuses to read into them
        bpl     :-
        lda     #$CF
        sta     $BF58
        lda     #$01
        sta     $BF6F
        jsr     MLI
        .byte   $C8             ; OPEN
        .addr   bgo
        bcs     @err
        lda     bgo+5
        sta     bgr+1
        sta     bgc+1
        jsr     MLI
        .byte   $CA             ; READ
        .addr   bgr
        php
        jsr     MLI
        .byte   $CC             ; CLOSE
        .addr   bgc
        plp
        bcs     @err
        ldy     #0              ; launch path at $0280
        lda     ($00),y
        tay
:       lda     ($00),y
        sta     $0280,y
        dey
        bpl     :-
        sei
        cld
        ldx     #$FF
        txs
        sta     $C000           ; 80STORE off
        sta     $C002           ; main memory for reading
        sta     $C004           ;   and writing
        sta     $C008           ; main zero page
        sta     $C051           ; text
        sta     $C054           ; page 1
        sta     $C056           ; lo-res
        sta     $C05F           ; AN3 on (no double hi-res)
        sta     $C00C           ; 40 columns
        bit     $C082           ; ROM
        cli
        jmp     $2000
@err:   lda     #1
        ldx     #0
        rts

bgo:    .byte   3
        .word   0               ; path
        .word   $0800           ; I/O buffer
        .byte   0
bgr:    .byte   4, 0
        .word   $2000, $A000 - $2000, 0
bgc:    .byte   1, 0

; ------------------------------------------------------------------------
; Quit code. Assembled for $1000, where ProDOS runs it.
_qstub:
        .org    $1000
qs:     cld
        ldx     #$FF
        txs
        bit     $C082           ; ROM
        ldx     #$17            ; reset the memory bitmap (BASIC.SYSTEM marks
:       stz     $BF58,x         ; its pages, which would stop the loader)
        dex
        bpl     :-
        lda     #$CF            ; zero page, stack, text page
        sta     $BF58
        lda     #$01            ; the global page
        sta     $BF6F
        jsr     MLI
        .byte   $C8             ; OPEN RW2.SYSTEM
        .addr   qopen
        bcs     qerr
        lda     qopen+5
        sta     qread+1
        sta     qclose+1
        jsr     MLI
        .byte   $CA             ; READ it to $2000
        .addr   qread
        jsr     MLI
        .byte   $CC
        .addr   qclose
        ldx     qpath           ; its path at $0280, for its folder
:       lda     qpath,x
        sta     $0280,x
        dex
        bpl     :-
        jmp     $2000
qerr:   jsr     $FC58           ; HOME
        ldx     #0
:       lda     qmsg,x
        beq     :+
        jsr     $FDED           ; COUT
        inx
        bne     :-
:       jsr     $FD0C           ; RDKEY
        inc     $03F4           ; spoil the power-up byte:
        jmp     ($FFFC)         ; reset restarts the computer
qopen:  .byte   3
        .word   qpath
        .word   $1C00
        .byte   0
qread:  .byte   4, 0
        .word   $2000, $1000, 0
qclose: .byte   1, 0
qmsg:   .byte   $D2,$D7,$B2,$AE,$D3,$D9,$D3,$D4,$C5,$CD,$A0     ; "RW2.SYSTEM "
        .byte   $CE,$CF,$D4,$A0,$C6,$CF,$D5,$CE,$C4,$AE,$A0     ; "NOT FOUND. "
        .byte   $D0,$D2,$C5,$D3,$D3,$A0,$C1,$A0,$CB,$C5,$D9     ; "PRESS A KEY"
        .byte   0
qpath:  .res    65
qend:
        .reloc

        .export         _qstub_len: absolute = qend - qs
        .export         _qpath_off: absolute = qpath - qs
