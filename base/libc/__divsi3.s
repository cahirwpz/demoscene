        xdef    ___divsi3
        xref    ___udivsi3

        section '.text',code

___divsi3:
        move.l  d2,-(sp)

        moveq.l #1,d2           ; sign of result stored in d2 (=1 or =-1)
        move.l  12(sp),d1       ; d1 = divisor
        bpl     L1
        neg.l   d1
        neg.b   d2              ; change sign because divisor < 0
L1:     move.l  8(sp),d0        ; d0 = dividend
        bpl     L2
        neg.l   d0
        neg.b   d2

L2:     move.l  d1,-(sp)
        move.l  d0,-(sp)
        bsr     ___udivsi3      ; divide abs(dividend) by abs(divisor)
        addq.l  #8,sp

        tst.b   d2
        bpl     L3
        neg.l   d0

L3:     move.l  (sp)+,d2
        rts

; vim: ft=asm68k:ts=8:sw=8
