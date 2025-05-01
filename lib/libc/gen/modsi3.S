#include <asm.h>

ENTRY(__modsi3)
	/* NB: this requires that __udivsi3 preserve a0 and return */
	/* the modulus in d1: */
	move.l	(sp)+,a0	/* pop return address */
	move.l	4(sp),d1	/* load the divisor */
	bpl.s	1f
	neg.l	4(sp)		/* store abs(divisor) */
1:	move.l	(sp),d0		/* load the dividend */
	pea	.Lret(pc)	/* push our return address */
	bpl.s	2f
	neg.l	4(sp)		/* store abs(dividend) */
	subq.l	#2,(sp)		/* adjust return address */
2:	jmp	_L(__udivsi3)
	neg.l	d1		/* negate modulus */
.Lret:	move.l	d1,d0		/* move modulus into d0 */
	jmp	(a0)
END(__modsi3)

# vim: ft=gas:ts=8:sw=8:noet:
