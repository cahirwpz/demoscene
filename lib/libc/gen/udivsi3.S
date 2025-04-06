/* Based on divsi3.S from NetBSD */

#include <asm.h>

ENTRY(__udivsi3)
	move.l	d2,-(sp)	/* save d2 */
	move.l	12(sp),d0	/* load divisor */
	move.l	8(sp),d1	/* load dividend */

/*
 * first, we divide the divisor and dividend by two until 
 * the divisor fits into 16 bits:
 */
1:	cmp.l	#0x10000,d0
	bcs.s	2f
	lsr.l	#1,d0
	lsr.l	#1,d1
	bra.s	1b
2:

/*
 * now we can do the divide.  to avoid overflow, we have to 
 * do the divide in two parts, high and low, and add the 
 * results together:
 */
	move.w	d1,d2		/* save low(dividend) */
	clr.w	d1
	swap	d1		/* d1 = dividend >> 16 */
	divu.w	d0,d1		/* do the high divide */
	move.l	d1,a1		/* save high divide result */
	move.w	d2,d1		/* concat(remainder, low(dividend)) */
	divu.w	d0,d1		/* do the low divide */
	move.l	a1,d0		/* recover high divide result */
	swap	d0
	clr.w	d0		/* d0 = finished high divide result */
	and.l	#0xffff,d1	/* d1 = finished low divide result */
	add.l	d1,d0		/* d0 = quotient guess */

/*
 * the quotient we have so far is only a guess.  the divide we 
 * did above was really the divide of some dividendB by some 
 * divisorB, where the following hold:
 *
 * (dividend - divisor) <= dividendB <= dividend
 * (divisor / 2) < divisorB <= divisor
 *
 * so our guess quotient cannot be less than our real desired
 * quotient.  however, it might be one too big.
 *
 * to adjust this quotient, we multiply it by the original 
 * divisor and subtract the result from the original dividend.  
 * if the result is nonnegative, our guessed quotient was 
 * correct, and the subtraction result is our remainder.  
 * if the result is negative, our guessed quotient was one 
 * too big, and the subtraction result plus the original 
 * divisor is our remainder.
 *
 * as in mulsi3, we have to do the multiply in stages to avoid 
 * overflow:
 */

	move.l	12(sp), d2	/* load divisor */
	swap	d2
	move.l	d0,d1
	mulu.w	d2,d1		/* high(divisor) * low(guess) */
	move.l	d1,a1		/* save high(divisor) * low(guess) */
	swap	d2
	move.l	d0,d1
	swap	d1
	mulu.w	d2,d1		/* low(divisor) * high(guess) */
	add.l	a1,d1
	swap	d1
	clr.w	d1		/* d1 = finished high multiply result */
	move.l	d2,a1		/* save original divisor */
	mulu.w	d0,d2		/* low(guess) * low(divisor) */
	add.l	d1,d2		/* d2 = guess * divisor */
	
	move.l	8(sp),d1	/* load original dividend */
	sub.l	d2,d1		/* subtract */
	bcc.s	3f
	subq.l	#1,d0		/* adjust quotient */
	add.l	a1,d1		/* adjust remainder */
3:	move.l	(sp)+,d2	/* restore d2 */
	rts
END(__udivsi3)

STRONG_ALIAS(ldivu,__udivsi3)

# vim: ft=gas:ts=8:sw=8:noet:
