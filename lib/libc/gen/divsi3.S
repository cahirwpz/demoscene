/* Based on divsi3.S from NetBSD */

#include <asm.h>

ENTRY(__divsi3)
	/* NB: this requires that __udivsi3 preserve a0 */
	move.l	4(sp),d1	/* load the dividend */
	bpl.s	1f
	neg.l	4(sp)		/* store abs(dividend) */
1:	move.l	8(sp),d0	/* load the divisor */
	bpl.s	2f
	neg.l	8(sp)		/* store abs(divisor) */
2:	eor.l	d1,d0
	bpl.s	3f		/* branch if sgn(divisor) == sgn(dividend) */
	move.l	(sp)+,a0	/* pop return address */
	pea	.Lret(pc)	/* push our return address */
3:	jmp	_L(__udivsi3)
.Lret:	neg.l	d0		/* negate quotient */
	jmp	(a0)
END(__divsi3)

STRONG_ALIAS(ldivs,__divsi3)

# vim: ft=gas:ts=8:sw=8:noet:
