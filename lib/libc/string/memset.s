; This code is covered by BSD license. It was taken from NetBSD
; common/lib/libc/arch/m68k/string/memset.S and converted to Motorola syntax.

        xdef    _memset

        section '.text',code

; [a0] destination
; [d1] count
; [d0] get fill character
_memset:
	move.l	d2,-(sp)
        move.b  d0,d2

	; It isn't worth the overhead of aligning to {long}word boundries
	; if the string is too short.
	cmp.l	#15,d1
	blt	.Lbzbyte

	clr.l   d0
	move.b	d2,d0
	move.l	d0,d2
	lsl.l	#8,d0
	or.l	d0,d2
	lsl.l	#8,d0	
	or.l	d0,d2	
	lsl.l	#8,d0	
	or.l    d0,d2	

	; word align
	move.l	a0,d0
	btst	#0,d0			; if (dst & 1)
	beq	.Lbzalgndw		; 
	move.b	d2,(a0)+		;	*(char *)dst++ = X
	subq.l	#1,d1			;	len--
	addq.l	#1,d0
.Lbzalgndw:
	; long word align
	btst	#1,d0			; if (dst & 2)
	beq	.Lbzalgndl		;
	move.w	d2,(a0)+		;	*(short *)dst++ = X
	subq.l	#2,d1			;	len -= 2
.Lbzalgndl:
	; set by 8 longwords
	move.l	d1,d0
	lsr.l	#5,d0			; cnt = len / 32
	beq	.Lbzlong		; if (cnt)
	and.l	#31,d1			;	len %= 32
	subq.l	#1,d0			;	set up for dbf
.Lbz32loop:
	move.l	d2,(a0)+		;	set 8 long words
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
	bcc	.Lbz32loop

.Lbzlong:
	; set by longwords
	move.l	d1,d0
	lsr.l	#2,d0			; cnt = len / 4
	beq	.Lbzbyte		; if (cnt)
	subq.l	#1,d0			;	set up for dbf
.Lbzlloop:
	move.l	d2,(a0)+		;	clear longwords
	dbf	d0,.Lbzlloop		;	till done
	clr.w	d0
	subq.l	#1,d0
	bcc	.Lbzlloop
	and.l	#3,d1			;	len %= 4
	beq	.Lbzdone

	subq.l	#1,d1			; set up for dbf
.Lbzbloop:
	move.b	d2,(a0)+		; set bytes
.Lbzbyte:
	dbf	d1,.Lbzbloop		; till done
.Lbzdone:
	move.l	8(sp),d0		; return destination
	move.l	(sp)+,d2
	rts

; vim: ft=asm68k:ts=8:sw=8:noet:
