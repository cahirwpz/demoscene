; This code is covered by BSD license. It was taken from NetBSD
; common/lib/libc/arch/m68k/string/bzero.S and converted to Motorola syntax.

        XDEF    _bzero

        SECTION '.text',code_p

; [a0] destination
; [d1] count
_bzero:
	move.l	d2,-(sp)
	clr.l	d2

	; It isn't worth the overhead of aligning to {long}word boundries
	; if the string is too short.
	cmp.l	#8,d1
	blt.b   .Lbzbyte

	; word align
	move.l	a0,d0
	btst	#0,d0			; if (dst & 1)
	beq.b	.Lbzalgndw		; 
	move.b	d2,(a0)+		; 	*(char *)dst++ = 0
	subq.l	#1,d1			;	len--
.Lbzalgndw:
	; long word align
	btst	#1,d0			; if (dst & 2)
	beq.s	.Lbzalgndl		;
	move.w	d2,(a0)+		;	*(short *)dst++ = 0
	subq.l	#2,d1			;	len -= 2
.Lbzalgndl:
	; zero by 8 longwords
	move.l	d1,d0
	lsr.l	#5,d0			; cnt = len / 32
	beq.b   .Lbzlong		; if (cnt)
	and.l	#31,d1			;	len %= 32
	subq.l	#1,d0			;	set up for dbf
.Lbz32loop:
	move.l	d2,(a0)+		;	zero 8 long words
	move.l	d2,(a0)+
	move.l	d2,(a0)+
	move.l	d2,(a0)+
	move.l	d2,(a0)+
	move.l	d2,(a0)+
	move.l	d2,(a0)+
	move.l	d2,(a0)+
	dbf	d0,.Lbz32loop		;	till done
	clr.w	d0
	subq.l	#1,d0
	bcc.b   .Lbz32loop

.Lbzlong:
	; copy by longwords
	move.l	d1,d0
	lsr.l	#2,d0			; cnt = len / 4
	beq.b	.Lbzbyte		; if (cnt)
	subq.l	#1,d0			;	set up for dbf
.Lbzlloop:
	move.l	d2,(a0)+		;	clear longwords
	dbf	d0,.Lbzlloop		;	till done
	and.l	#3,d1			;	len %= 4
	beq.s	.Lbzdone

	subq.l	#1,d1			; set up for dbf
.Lbzbloop:
	move.b	d2,(a0)+		; zero bytes
.Lbzbyte:
	dbf	d1,.Lbzbloop		; till done
.Lbzdone:
	move.l	(sp)+,d2
	rts

; vim: ft=asm68k:ts=8:sw=8
