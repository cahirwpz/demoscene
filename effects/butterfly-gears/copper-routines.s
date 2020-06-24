; vim: ft=asm68k:ts=8:sw=8:

	include 'hardware/custom.i'

DEBUG_TIMING			= 0
ROTZOOM_W			= 24
ROTZOOM_H			= 24
COPPER_HALFROW_INSTRUCTIONS	= ROTZOOM_W/2+4
SINGLEROW_SIZE			= COPPER_HALFROW_INSTRUCTIONS*4

Y0				= (256-280)/2+$2c
STATIC_Y_AREA			= 40
BOOK_Y				= 256
STATIC_Y_START			= BOOK_Y-STATIC_Y_AREA
COPWAIT_X			= 0
BALL_PADDING_TOP		= 6
BALL_PADDING_BOTTOM		= 11
NO_OP				= $1fe
LARGE_BALL_HEIGHT		= 112

DEBUG_COL macro
	if DEBUG_TIMING=1
	move.w	#\1,$dff180
	endif
	endm

COPJUMP	macro
	move.w	#cop2lc+2,(\2)+
	move.w	\1,(\2)+
	swap	\1
	move.w	#cop2lc,(\2)+
	move.w	\1,(\2)+
	move.w	#copjmp2,(\2)+
	endm

	xdef	_PlotTextureAsm
	xdef	_CopyTextureAsm
	xdef	_UpdateBallCopper
	xdef	_WriteStaticYArea

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
; extern void PlotTextureAsm(char *copperDst asm("a0"),
;                            char *texture   asm("a1"),
;                            int  u          asm("d0"),
;                            int  v          asm("d2"),
;                            int  uDeltaCol  asm("d1"),
;                            int  vDeltaCol  asm("d3"),
;                            int  uDeltaRow  asm("d5"),
;                            int  vDeltaRow  asm("d6"));
; ---------------------------------------------------------------------------------------

_PlotTextureAsm:
	movem.l	d0-a6,-(a7)
	move.l	#4711,d4
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

	DEBUG_COL $245
	bsr.b	tx_rotzoom
	DEBUG_COL $134

	movem.l	(a7)+,d0-a6
	rts

; ---------------------------------------------------------------------------------------

	include	data/textureloop-generated.s

; ---------------------------------------------------------------------------------------
; extern void CopyTextureAsm(char *copperSrc asm("a0"), char *copperDst asm("a1"));
; ---------------------------------------------------------------------------------------
_CopyTextureAsm:
	DEBUG_COL $023
	bsr	tx_copy
	DEBUG_COL $134
	rts

; ---------------------------------------------------------------------------------------
; extern void UpdateBallCopper(int     y              asm("d0"),
;                              int     yInc           asm("d1"),
;                              CopInsT *staticYSource asm("a0"),
;                              CopInsT *textureCopper asm("a1"),
;                              CopInsT *paddingTop    asm("a2"),
;                              CopInsT *paddingBottom asm("a3"));
; ---------------------------------------------------------------------------------------

; Update the non-texture parts of a ball copperlist.
;
; - Update Y waits
; - Weave in commands from the static Y area (so we get "stable" Y positions even though the ball is moving)

_UpdateBallCopper:
	DEBUG_COL $136
	cmp.w	#STATIC_Y_START-LARGE_BALL_HEIGHT-3,d0
	blt.w	_UpdateBallCopper_fasttrack

	; -------------------------------------------------------------------------------
	; padding area above the ball
	; -------------------------------------------------------------------------------

	move.w	#BALL_PADDING_TOP-1,d2
.top	addq	#1,d0			; y++

	moveq	#0,d3
	cmp.w	#Y0,d0			; wait for 0 if above top or at book area start
	ble.b	.topy0
	cmp.w	#BOOK_Y-1,d0
	bge.b	.topy0
	move.w	d0,d3
.topy0	move.b	d3,(a2)+
	add.w	#3,a2

	cmp.w	#BOOK_Y-1,d0
	ble.b	.no_top_abort
	move.w	#copjmp2,(a2)
	DEBUG_COL $134
	rts
.no_top_abort
	move.l	(a0)+,(a2)+
	dbf	d2,.top

	; -------------------------------------------------------------------------------
	; ball
	; -------------------------------------------------------------------------------

	move.w	#ROTZOOM_H*2-1,d2
.ball	move.w	d0,d3			; wait for 0 if above top
	cmp.w	#Y0,d0
	bgt.b	.ball_y_write
	moveq	#0,d3
.ball_y_write
	move.b	d3,(a1)

	add.w	#(ROTZOOM_W/2+1)*4,a1	; skip after texture writes
	add.w	d1,d0			; y += yInc

	cmp.w	#BOOK_Y-1,d0		; abort at BOOK_Y
	blt.b	.no_ball_abort
	move.w	#copjmp2,(a1)
	DEBUG_COL $134
	rts

.no_ball_abort
	move.l	(a0)+,(a1)+		; after texture writes: command for static Y area

	cmp.w	#1,d1			; then, either two NOPs (small ball)
	bne.b	.big_ball
	move.w	#NO_OP,(a1)+
	addq	#2,a1
	move.w	#NO_OP,(a1)+
	addq	#2,a1
	bra.b	.extra_done

.big_ball
	move.w	d0,d3			; or in-between-Y wait + another command (big ball)
	subq	#1,d3
	cmp.w	#Y0,d0
	bgt.b	.keepy
	moveq	#0,d3
.keepy	move.b	d3,(a1)+
	move.b	#COPWAIT_X|1,(a1)+
	move.w	#$ff00,(a1)+
	move.l	(a0)+,(a1)+
.extra_done
	dbf	d2,.ball

	; -------------------------------------------------------------------------------
	; padding area below ball
	; -------------------------------------------------------------------------------

	move.w	#BALL_PADDING_BOTTOM-1,d2
.bottom
	moveq	#0,d3
	cmp.w	#Y0,d0			; wait for 0 if above top or at book area start
	ble.b	.boty0
	cmp.w	#BOOK_Y-1,d0
	bge.b	.boty0
	move.w	d0,d3
.boty0	move.b	d3,(a3)+
	add.w	#3,a3

	cmp.w	#BOOK_Y-1,d0
	ble.b	.no_bottom_abort
	move.w	#copjmp2,(a3)
	DEBUG_COL $134
	rts
.no_bottom_abort
	move.l	(a0)+,(a3)+
	addq	#1,d0
	dbf	d2,.bottom
	DEBUG_COL $134
	rts

; ---------------------------------------------------------------------------------------
; same as above, without static Y command weaving (ball is too high on screen)
; ---------------------------------------------------------------------------------------
_UpdateBallCopper_fasttrack
	move.w	#BALL_PADDING_TOP-1,d2
.top	addq	#1,d0			; y++
	moveq	#0,d3
	cmp.w	#Y0,d0			; wait for 0 if above top or at book area start
	ble.b	.topy0
	cmp.w	#BOOK_Y-1,d0
	bge.b	.topy0
	move.w	d0,d3
.topy0	move.b	d3,(a2)+
	add.w	#3,a2
	cmp.w	#BOOK_Y-1,d0
	ble.b	.no_top_abort
	move.w	#copjmp2,(a2)
	rts
.no_top_abort
	;;;;move.l	(a0)+,(a2)+
	addq	#4,a2
	dbf	d2,.top
	move.w	#ROTZOOM_H*2-1,d2
.ball	move.w	d0,d3			; wait for 0 if above top
	cmp.w	#Y0,d0
	bgt.b	.ball_y_write
	moveq	#0,d3
.ball_y_write
	move.b	d3,(a1)
	add.w	#(ROTZOOM_W/2+1)*4,a1	; skip after texture writes
	add.w	d1,d0			; y += yInc
	cmp.w	#BOOK_Y-1,d0		; abort at BOOK_Y
	blt.b	.no_ball_abort
	move.w	#copjmp2,(a1)
	rts
.no_ball_abort
	;;;;move.l	(a0)+,(a1)+		; after texture writes: command for static Y area
	addq	#4,a1
	cmp.w	#1,d1			; then, either two NOPs (small ball)
	bne.b	.big_ball
	;;;;move.w	#NO_OP,(a1)+
	;;;;addq	#2,a1
	;;;;move.w	#NO_OP,(a1)+
	;;;;addq	#2,a1
	addq	#8,a1
	bra.b	.extra_done
.big_ball
	move.w	d0,d3			; or in-between-Y wait + another command (big ball)
	subq	#1,d3
	cmp.w	#Y0,d0
	bgt.b	.keepy
	moveq	#0,d3
.keepy	move.b	d3,(a1)+
	;;;;move.b	#COPWAIT_X|1,(a1)+
	;;;;move.w	#$ff00,(a1)+
	;;;;move.l	(a0)+,(a1)+
	addq	#7,a1
.extra_done
	dbf	d2,.ball
	move.w	#BALL_PADDING_BOTTOM-1,d2
.bottom
	moveq	#0,d3
	cmp.w	#Y0,d0			; wait for 0 if above top or at book area start
	ble.b	.boty0
	cmp.w	#BOOK_Y-1,d0
	bge.b	.boty0
	move.w	d0,d3
.boty0	move.b	d3,(a3)+
	add.w	#3,a3
	cmp.w	#BOOK_Y-1,d0
	ble.b	.no_bottom_abort
	move.w	#copjmp2,(a3)
	rts
.no_bottom_abort
	;;;;move.l	(a0)+,(a3)+
	addq	#4,a3
	addq	#1,d0
	dbf	d2,.bottom
	rts

; ---------------------------------------------------------------------------------------
; extern void WriteStaticYArea(CopInsT *copperDst        asm("a0"),
;                              CopInsT *copperSrc        asm("a1"),
;                              int     bottomBallY       asm("d0"),
;                              CopInsT *bottomBallCopper asm("a2"),
;                              CopInsT *bookCopper       asm("a3"));
; ---------------------------------------------------------------------------------------

; Write the copper area above the book, possibly containing a copper jump if a ball is
; partly visible before the book "cut" at line BOOK_Y occurs.
;
; - Create copper list for STATIC_Y_START to BOOK_Y
; - Read copper instructions from copperSrc and write pairs of (wait Y, instruction) into copperDst
; - If Y reaches bottomBallY, abort and copper-jump to bottomBallCopper
; - Otherwise, copper-jump to bookCopper at the end

_WriteStaticYArea:
	movem.l	d0-a6,-(a7)

	move.w	#STATIC_Y_START-1,d1
.copy	addq	#1,d1
	cmp.w	d0,d1			; Y == bottomBallY ?
	beq.b	.jumpAbort

	move.b	d1,(a0)+		; write copper wait
	move.b	#COPWAIT_X|1,(a0)+
	move.w	#$fffe,(a0)+
	move.l	(a1)+,(a0)+		; write instruction

	cmp.w	#BOOK_Y,d1
	blt.b	.copy

	move.l	a3,d0
	COPJUMP	d0,a0

	movem.l	(a7)+,d0-a6
	rts

.jumpAbort
	move.l	a2,d0
	COPJUMP	d0,a0
	movem.l	(a7)+,d0-a6
	rts
