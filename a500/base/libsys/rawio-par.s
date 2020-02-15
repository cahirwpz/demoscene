        include 'hardware/cia.i'

        xdef    _DPutChar
        xdef    _DPutByte
        xdef    _DPutWord
        xdef    _DPutLong
        xdef    _DPutStr

        section '.text',code

; Reliable transfers may only happen 3 or more E-cycles basis.
; http://lallafa.de/blog/2015/09/amiga-parallel-port-how-fast-can-you-go/

_DPutChar:
        move.l  a0,-(sp)

        lea     $bfd000,a0              ; [a0] ciab, ciaa = ciab + $1001

        tst.b   _parallel
        beq     .check
        blt     .quit

.loop   btst    #CIAB_PRTRBUSY,(a0)     ; is printer buffer empty ?
        bne     .loop

        move.b  #$ff,ciaddrb+$1001(a0)  ; set direction to out
        move.b  d0,ciaprb+$1001(a0)     ; write the data
        tst.b   (a0)                    ; waste 2 E-cycles
        tst.b   (a0)

        cmp.b   #10,d0  ; '\n'
        bne     .quit

        moveq   #13,d0  ; '\r'
        bra     .loop

.quit   move.l  (sp)+,a0
        rts

.check  move.b  #1,_parallel

        btst    #CIAB_PRTRBUSY,(a0)
        beq     .loop

.noprt  move.b  #-1,_parallel
        bra     .quit

_DPutByte:
        ror.l   #8,d0
        moveq   #1,d1
        bra     DPutInt

_DPutWord:
        swap    d0
        moveq   #3,d1
        bra     DPutInt

_DPutLong:
        moveq   #7,d1

DPutInt:
        lea     hex(pc),a0

.loop   rol.l   #4,d0
        move.l  d0,-(sp)
        and.w   #$000f,d0
        move.b  (a0,d0.w),d0
        bsr     _DPutChar
        move.l  (sp)+,d0
        dbf     d1,.loop

        rts

hex:    dc.b    '0123456789ABCDEF'

_DPutStr:
.loop   move.b  (a0)+,d0
        bne     .putc
        rts

.putc   bsr     _DPutChar
        bra     .loop

        section '.bss',bss

_parallel:
        ds.b    1

; vim: ft=asm68k:ts=8:sw=8
