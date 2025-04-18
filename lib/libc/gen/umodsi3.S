/* Based on umodsi3.S from NetBSD */

#include <asm.h>

ENTRY(__umodsi3)
	/*
	 * NB: this requires that __udivsi3 preserve the a0
         * register, and that it returns the modulus in d1.
	 */
	move.l	(sp)+,a0	/* pop the return address */
	bsr	_L(__udivsi3)
	move.l	d1,d0		/* move the modulus into d0 */
	jmp	(a0)		/* return */
END(__umodsi3)

# vim: ft=gas:ts=8:sw=8:noet:
