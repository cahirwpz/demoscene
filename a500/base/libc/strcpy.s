; This code is covered by BSD license. It was taken from NetBSD
; common/lib/libc/arch/m68k/string/strlen.S and converted to Motorola syntax.

        xdef    _strcpy

        section '.text',code

_strcpy:
	move.l	8(sp),a0		; a0 = fromaddr
	move.l	4(sp),d0		; return value is toaddr
	move.l	d0,a1			; a1 = toaddr
.Lscloop:
	move.b	(a0)+,(a1)+		; copy a byte
	bne	.Lscloop		; copied non-null, keep going
	rts

; vim: ft=asm68k:ts=8:sw=8:noet:
