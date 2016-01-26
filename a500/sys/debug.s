        xdef    _KPutChar
        xdef    _KPutByte
        xdef    _KPutWord
        xdef    _KPutLong
        xdef    _KPutStr
        xdef    _CallHRTmon

        include 'hardware/custom.i'

        section "code",code

_KPutChar:
        and.w   #$ff,d0
        or.w    #$100,d0

.loop   btst    #5,serdatr+$dff000
        beq     .loop

        move.w  d0,serdat+$dff000

        cmp.b   #10,d0  ; '\n'
        bne     .quit

        addq.w  #3,d0   ; '\r'
        bra     .loop

.quit:  rts

_KPutByte:
        ror.l   #8,d0
        moveq   #1,d1
        bra     KPutInt

_KPutWord:
        swap    d0
        moveq   #3,d1
        bra     KPutInt

_KPutLong:
        moveq   #7,d1

KPutInt:
        lea     hex(pc),a0

.loop   rol.l   #4,d0
        move.l  d0,-(sp)
        and.w   #$000f,d0
        move.b  (a0,d0.w),d0
        bsr     _KPutChar
        move.l  (sp)+,d0
        dbf     d1,.loop

        rts

hex:    dc.b    '0123456789ABCDEF'

_KPutStr:
.loop   move.b  (a0)+,d0
        bne     .putc
        rts

.putc   bsr     _KPutChar
        bra     .loop

_CallHRTmon:
        lea     4(sp),sp
        jmp     $a1000c

; vim: ft=asm68k:ts=8:sw=8
