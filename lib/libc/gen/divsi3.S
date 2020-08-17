#include <asm.h>

ENTRY(__divsi3)
        move.l  d2,-(sp)

        moveq.l #1,d2		/* sign of result stored in d2 (=1 or =-1) */
        move.l  12(sp),d1	/* d1 = divisor */
        jpl     .L1
        neg.l   d1
        neg.b   d2		/* change sign because divisor < 0 */
.L1:    move.l  8(sp),d0	/* d0 = dividend */
        jpl     .L2
        neg.l   d0
        neg.b   d2

.L2:    move.l  d1,-(sp)
        move.l  d0,-(sp)
        jsr     _L(__udivsi3)   /* divide abs(dividend) by abs(divisor) */
        addq.l  #8,sp

        tst.b   d2
        jpl     .L3
        neg.l   d0

.L3:    move.l  (sp)+,d2
        rts
END(__divsi3)

STRONG_ALIAS(__ldivs,__divsi3)

# vim: ft=gas:ts=8:sw=8:noet:
