;
; Written by Olaf `Olsen' Barthel <olsen@sourcery.han.de> 
; Modified by Krystian Bacławski
; Public Domain
;

	include	"exec/execbase.i"
	include	"exec/tasks.i"
        include "lvo/exec_lib.i"

        xref    _KPutChar
	xdef	_TrapHandler

	section	text,code

_TrapHandler:
	move.l	a0,-(sp)		; save A0, we are going to need it in a minute
	move.l	usp,a0			; as we are in supervisor mode this minute, this gets the user stack pointer
	movem.l	d0-d7/a0-a7,-(a0)	; push all registers on the user stack; A0 and A7 will need fixing
	move.l	(sp)+,8*4(a0)		; push the original contents of A0 into the stack
	move.l	a0,d0			; save this for later
	add.l	#4*16,d0		; fix the stack offset
	move.l	d0,(8+7)*4(a0)		; and push the correct user stack pointer into the stack

	move.l	$4.w,a6		        ; get the ExecBase pointer, we are going to need it in a minute

	move.l	(sp)+,d2		; get the type of the trap
	cmp.l	#3,d2			; was it an address error?
	bne.b	.no_address_error	; skip the following if it isn't

	move.w	AttnFlags(a6),d0
	andi.w	#AFF_68010,d0		; is this a plain 68k machine?
	bne.b	.no_68k			; skip the following if it isn't

	addq.l	#8,sp			; skip the extra information the 68k puts into
					; the address error exception frame

.no_address_error:
.no_68k:
	moveq	#0,d4
	move.w	(sp)+,d4		; get the copy of the status register
	move.l	(sp)+,d3	        ; get the program counter of the offending command

        movem.l d2-d4,-(a0) 
        move.l  a0,a1

        lea     .message(pc),a0
        lea     _KPutChar,a2
        suba.l  a3,a3

        move.l  $4.w,a6
        jsr     _LVORawDoFmt(a6)

        stop    #$2700

.message:
        dc.b    'Trap: $%08lx PC: $%08lx SR: $%04lx\n'
        dc.b    'D0: %08lx D1: %08lx D2: %08lx D3: %08lx\n'
        dc.b    'D4: %08lx D5: %08lx D6: %08lx D7: %08lx\n'
        dc.b    'A0: %08lx A1: %08lx A2: %08lx A3: %08lx\n'
        dc.b    'A4: %08lx A5: %08lx A6: %08lx SP: %08lx\n'
        dc.b    0

; vim: ft=asm68k:ts=8:sw=8
