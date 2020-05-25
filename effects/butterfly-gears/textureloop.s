; vim: ft=asm68k:ts=8:sw=8:

ROTZOOM_W			= 24
ROTZOOM_H			= 24
COPPER_HALFROW_INSTRUCTIONS	= ROTZOOM_W/2+2
SINGLEROW_SIZE			= COPPER_HALFROW_INSTRUCTIONS*4

	xdef _PlotTextureAsm

	section	'.text',code

	macro PREPARE_UV
	;
	; [\1] ................ -VVVVVVVvvvvvvvv
	; [\2] ................ -UUUUUUUuuuuuuuu
	;
	move.w	\1,(a2)		; a2 VVvv
	swap	\2		;  2 UUuu....
	clr.w	\2		;  2 UUuu----
	move.b	\1,\2		;  2 UUuu--vv
	lsl.w	#8,\2		;  2 UUuuvv--
	swap	\2		;  2 vv--UUuu
	moveq	#0,\1		;  1 --------
	move.b	(a2),\1		;  1 ------VV
	;
	; [\1] ................ ........-VVVVVVV
	; [\2] vvvvvvvv........ -UUUUUUUuuuuuuuu
	;
	endm

; ---------------------------------------------------------------------------------------
; _PlotTextureAsm(copperDst, texture, u,  v,  uDeltaCol, vDeltaCol, uDeltaRow, vDeltaRow)
;                 a0         a1       d0  d2, d1         d3,        d5,        d6
; ---------------------------------------------------------------------------------------

_PlotTextureAsm:
	movem.l	d4/d7/a2,-(a7)
	addq.w	#4+2,a0		; first color write data word
	add.l	#128*128*2,a1	; point to middle of texture

	PREPARE_UV d2,d0 ; [d0] [d2] = position
	PREPARE_UV d3,d1 ; [d1] [d3] = delta col
	PREPARE_UV d6,d5 ; [d5] [d6] = delta row

	move.l	d5,a2	; [a2] = delta row instead of d5
	move.l	d0,d5	; [d5] = copy of d0 (to store row pos)
	move.l	d2,d7	; [d7] = copy of d7 (to store row pos)

	; [d0] value        | vvvvvvvv........ -UUUUUUUuuuuuuuu
	; [d2] value        | ................ ........-VVVVVVV
	; [d1] delta column | vvvvvvvv........ -UUUUUUUuuuuuuuu
	; [d3] delta column | ................ ........-VVVVVVV
	; [a2] delta row    | vvvvvvvv........ -UUUUUUUuuuuuuuu
	; [d6] delta row    | ................ ........-VVVVVVV
	; [d4] scratch (plot offset)
	; [d5] scratch (save d0)
	; [d7] scratch (save d2)

	move #$245,$dff180
	include	data/textureloop-generated.s
	move #$134,$dff180

	movem.l	(a7)+,d4/d7/a2
	rts
