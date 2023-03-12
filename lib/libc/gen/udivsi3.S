#include <asm.h>

ENTRY(__udivsi3)
        move.l  d2,-(sp)
        move.l  12(sp),d1       /* d1 = divisor */
        move.l  8(sp),d0        /* d0 = dividend */

        cmp.l   #0x10000,d1     /* divisor >= 2 ^ 16 ? */
        jcc     .L3             /* then try next algorithm */
        move.l  d0,d2
        clr.w   d2
        swap    d2
        divu.w  d1,d2           /* high quotient in lower word */
        move.w  d2,d0           /* save high quotient */
        swap    d0
        move.w  10(sp),d2       /* get low dividend + high rest */
        divu    d1,d2           /* low quotient */
        move.w  d2,d0
        jra     .L6

.L3:    move.l  d1,d2           /* use d2 as divisor backup */
.L4:    lsr.l   #1,d1           /* shift divisor */
        lsr.l   #1,d0           /* shift dividend */
        cmp.l   #0x10000,d1     /* still divisor >= 2 ^ 16 ? */
        jcc     .L4
        divu.w  d1,d0           /* now we have 16 bit divisor */
        and.l   #0xffff,d0      /* mask out divisor, ignore remainder */

# Multiply the 16 bit tentative quotient with the 32 bit divisor.  Because of
# the operand ranges, this might give a 33 bit product.  If this product is
# greater than the dividend, the tentative quotient was too large.

        move.l  d2,d1
        mulu.w  d0,d1           /* low part, 32 bits */
        swap    d2
        mulu.w  d0,d2           /* high part, at most 17 bits */
        swap    d2              /* align high part with low part */
        tst.w   d2              /* high part 17 bits? */
        jne     .L5             /* if 17 bits, quotient was too large */
        add.l   d2,d1           /* add parts */
        jcs     .L5             /* if sum is 33 bits, quotient was too large */
        cmp.l   8(sp),d1        /* compare the sum with the dividend */
        jls     .L6             /* if sum > dividend, quotient was too large */
.L5:    subq.l  #1,d0           /* adjust quotient */

.L6:    move.l  (sp)+,d2
        rts

STRONG_ALIAS(__ldivu,__udivsi3)

# vim: ft=gas:ts=8:sw=8:noet:
