        XDEF    ___umodsi3
        XREF    ___udivsi3
        XREF    ___mulsi3

        SECTION "__umodsi3",CODE

___umodsi3:
        move.l  8(sp),d1        ; d1 = divisor
        move.l  4(sp),d0        ; d0 = dividend
        move.l  d1,-(sp)
        move.l  d0,-(sp)
        bsr     ___udivsi3
        addq.l  #8,sp
        move.l  8(sp),d1        ; d1 = divisor
        move.l  d1,-(sp)
        move.l  d0,-(sp)
        bsr     ___mulsi3       ; d0 = (a / b) * b
        addq.l  #8,sp
        move.l  4(sp),d1        ; d1 = dividend
        sub.l   d0,d1           ; d1 = a - (a / b) * b
        move.l  d1,d0
        rts

; vim: ft=asm68k:ts=8:sw=8
