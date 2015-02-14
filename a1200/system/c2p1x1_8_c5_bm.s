;
; File:   c2p1x1_8_c5_bm_040.s
; Author: Mikael Kalms <mikael@kalms.org>
; Date:   17 April 2000
; Title:  C2P - 1x1, 8bpl, BitMap output, 040+ optimized
;
; Description:
;   Performs CPU-only C2P conversion
;   Outputs to any BitMap (interlaced, large, ...)
;   Position-independent (PC-relative) code
;   68040+ optimized -- performs badly on 020/030
;   No selfmodifying code used (relies on datacache instead)
;   For best performance, align chunkybuffer and destination window
;     on even 4byte boundary
;
; Restrictions:
;   Chunky-buffer must be an even multiple of 32 pixels wide
;   X-Offset must be set to an even multiple of 8
;

	xdef	_c2p1x1_8_c5_bm

	incdir	include:
	include	graphics/gfx.i


				rsreset
C2P1X1_8_C5_BM_040_DELTA1	rs.l	1
C2P1X1_8_C5_BM_040_DELTA2	rs.l	1
C2P1X1_8_C5_BM_040_DELTA3	rs.l	1
C2P1X1_8_C5_BM_040_DELTA4	rs.l	1
C2P1X1_8_C5_BM_040_DELTA5	rs.l	1
C2P1X1_8_C5_BM_040_DELTA6	rs.l	1
C2P1X1_8_C5_BM_040_DELTA7	rs.l	1
C2P1X1_8_C5_BM_040_DELTA8	rs.l	1
C2P1X1_8_C5_BM_040_CHUNKYX	rs.w	1
C2P1X1_8_C5_BM_040_CHUNKYY	rs.w	1
C2P1X1_8_C5_BM_040_ROWMOD	rs.l	1
C2P1X1_8_C5_BM_040_SIZEOF	rs.b	0


	section	c2p1x1_8_c5_bm_040,code

; d0.w	chunkyx [chunky-pixels]
; d1.w	chunkyy [chunky-pixels]
; d2.w	offsx [screen-pixels]
; d3.w	offsy [screen-pixels]
; a0	chunkyscreen
; a1	BitMap

_c2p1x1_8_c5_bm:
	movem.l	d2-d7/a2-a6,-(sp)
	sub.l	#C2P1X1_8_C5_BM_040_SIZEOF,sp
					; A few sanity checks
	cmpi.b	#8,bm_Depth(a1)		; At least 8 valid bplptrs?
	blo	.exit
	move.w	d0,d4
	move.w	d2,d5
	andi.w	#$1f,d4			; Even 32-pixel width?
	bne	.exit
	andi.w	#$7,d5			; Even 8-pixel xoffset?
	bne	.exit
	moveq	#0,d4
	move.w	bm_BytesPerRow(a1),d4

	move.w	d0,C2P1X1_8_C5_BM_040_CHUNKYX(sp) ; Skip if 0 pixels to convert
	beq	.exit
	move.w	d1,C2P1X1_8_C5_BM_040_CHUNKYY(sp)
	beq	.exit

	and.l	#$ffff,d0
	and.l	#$ffff,d2

	mulu.w	d4,d3			; Offs to first pixel to draw in bpl
	lsr.l	#3,d2
	add.l	d2,d3

	lsl.l	#3,d4			; Positive modulo?
	sub.l	d0,d4
	bmi	.exit

	lsr.l	#3,d4
	move.l	d4,C2P1X1_8_C5_BM_040_ROWMOD(sp) ; Modulo between two rows

	move.l	a0,a2			; Ptr to end of line
	add.w	C2P1X1_8_C5_BM_040_CHUNKYX(sp),a2

	move.l	bm_Planes+1*4(a1),a3
	sub.l	bm_Planes+5*4(a1),a3
	move.l	a3,C2P1X1_8_C5_BM_040_DELTA1(sp)
	move.l	bm_Planes+4*4(a1),a3
	sub.l	bm_Planes+1*4(a1),a3
	move.l	a3,C2P1X1_8_C5_BM_040_DELTA2(sp)
	move.l	bm_Planes+0*4(a1),a3
	sub.l	bm_Planes+4*4(a1),a3
	move.l	a3,C2P1X1_8_C5_BM_040_DELTA3(sp)
	move.l	bm_Planes+7*4(a1),a3
	sub.l	bm_Planes+0*4(a1),a3
	addq.l	#4,a3
	move.l	a3,C2P1X1_8_C5_BM_040_DELTA4(sp)
	move.l	bm_Planes+3*4(a1),a3
	sub.l	bm_Planes+7*4(a1),a3
	move.l	a3,C2P1X1_8_C5_BM_040_DELTA5(sp)
	move.l	bm_Planes+6*4(a1),a3
	sub.l	bm_Planes+3*4(a1),a3
	move.l	a3,C2P1X1_8_C5_BM_040_DELTA6(sp)
	move.l	bm_Planes+2*4(a1),a3
	sub.l	bm_Planes+6*4(a1),a3
	move.l	a3,C2P1X1_8_C5_BM_040_DELTA7(sp)
	move.l	bm_Planes+5*4(a1),a3
	sub.l	bm_Planes+2*4(a1),a3
	move.l	a3,C2P1X1_8_C5_BM_040_DELTA8(sp)

	move.l	bm_Planes+7*4(a1),a1
	add.l	d3,a1


	tst.b	32(a0)
	move.l	(a0)+,d0
	move.l	(a0)+,d1
	move.l	(a0)+,d2
	move.l	(a0)+,d3
	tst.b	32(a0)
	move.l	(a0)+,d4
	move.l	(a0)+,d5
	move.l	(a0)+,a5
	move.l	(a0)+,a6

	swap	d4			; Swap 16x4, part 1
	swap	d5
	eor.w	d0,d4
	eor.w	d1,d5
	eor.w	d4,d0
	eor.w	d5,d1
	eor.w	d0,d4
	eor.w	d1,d5
	swap	d4
	swap	d5

	move.l	d4,d6			; Swap 2x4, part 1
	move.l	d5,d7
	lsr.l	#2,d6
	lsr.l	#2,d7
	eor.l	d0,d6
	eor.l	d1,d7
	and.l	#$33333333,d6
	and.l	#$33333333,d7
	eor.l	d6,d0
	eor.l	d7,d1
	lsl.l	#2,d6
	lsl.l	#2,d7
	eor.l	d6,d4
	eor.l	d7,d5

	exg	d4,a5
	exg	d5,a6

	swap	d4			; Swap 16x4, part 2
	swap	d5
	eor.w	d2,d4
	eor.w	d3,d5
	eor.w	d4,d2
	eor.w	d5,d3
	eor.w	d2,d4
	eor.w	d3,d5
	swap	d4
	swap	d5

	move.l	d4,d6			; Swap 2x4, part 2
	move.l	d5,d7
	lsr.l	#2,d6
	lsr.l	#2,d7
	eor.l	d2,d6
	eor.l	d3,d7
	and.l	#$33333333,d6
	and.l	#$33333333,d7
	eor.l	d6,d2
	eor.l	d7,d3
	lsl.l	#2,d6
	lsl.l	#2,d7
	eor.l	d6,d4
	eor.l	d7,d5

	move.l	d1,d6			; Swap 4x1, part 1
	move.l	d3,d7
	lsr.l	#4,d6
	lsr.l	#4,d7
	eor.l	d0,d6
	eor.l	d2,d7
	and.l	#$0f0f0f0f,d6
	and.l	#$0f0f0f0f,d7
	eor.l	d6,d0
	eor.l	d7,d2
	lsl.l	#4,d6
	lsl.l	#4,d7
	eor.l	d6,d1
	eor.l	d7,d3

	move.l	d2,d6			; Swap 8x2, part 1
	move.l	d3,d7
	lsr.l	#8,d6
	lsr.l	#8,d7
	eor.l	d0,d6
	eor.l	d1,d7
	and.l	#$00ff00ff,d6
	and.l	#$00ff00ff,d7
	eor.l	d6,d0
	eor.l	d7,d1
	lsl.l	#8,d6
	lsl.l	#8,d7
	eor.l	d6,d2
	eor.l	d7,d3

	bra	.xstart

.y
.x
	tst.b	32(a0)
	move.l	(a0)+,d0
	move.l	(a0)+,d1
	move.l	(a0)+,d2
	move.l	(a0)+,d3
	tst.b	32(a0)
	move.l	(a0)+,d4
	move.l	(a0)+,d5
	move.l	(a0)+,a5
	move.l	(a0)+,a6

	move.l	d6,(a1)

	swap	d4			; Swap 16x4, part 1
	swap	d5
	eor.w	d0,d4
	eor.w	d1,d5
	eor.w	d4,d0
	eor.w	d5,d1
	eor.w	d0,d4
	add.l	C2P1X1_8_C5_BM_040_DELTA1(sp),a1
	eor.w	d1,d5
	swap	d4
	swap	d5

	move.l	d4,d6			; Swap 2x4, part 1
	move.l	d7,(a1)
	move.l	d5,d7
	lsr.l	#2,d6
	lsr.l	#2,d7
	eor.l	d0,d6
	eor.l	d1,d7
	and.l	#$33333333,d6
	and.l	#$33333333,d7
	eor.l	d6,d0
	eor.l	d7,d1
	lsl.l	#2,d6
	lsl.l	#2,d7
	eor.l	d6,d4
	eor.l	d7,d5

	exg	d4,a5
	add.l	C2P1X1_8_C5_BM_040_DELTA2(sp),a1
	exg	d5,a6

	swap	d4			; Swap 16x4, part 2
	swap	d5
	eor.w	d2,d4
	eor.w	d3,d5
	eor.w	d4,d2
	eor.w	d5,d3
	eor.w	d2,d4
	eor.w	d3,d5
	swap	d4
	swap	d5

	move.l	a3,(a1)
	move.l	d4,d6			; Swap 2x4, part 2
	move.l	d5,d7
	lsr.l	#2,d6
	lsr.l	#2,d7
	eor.l	d2,d6
	eor.l	d3,d7
	and.l	#$33333333,d6
	and.l	#$33333333,d7
	eor.l	d6,d2
	eor.l	d7,d3
	lsl.l	#2,d6
	lsl.l	#2,d7
	eor.l	d6,d4
	eor.l	d7,d5

	move.l	d1,d6			; Swap 4x1, part 1
	move.l	d3,d7
	lsr.l	#4,d6
	lsr.l	#4,d7
	eor.l	d0,d6
	eor.l	d2,d7
	and.l	#$0f0f0f0f,d6
	and.l	#$0f0f0f0f,d7
	add.l	C2P1X1_8_C5_BM_040_DELTA3(sp),a1
	eor.l	d6,d0
	eor.l	d7,d2
	lsl.l	#4,d6
	lsl.l	#4,d7
	eor.l	d6,d1
	move.l	a4,(a1)
	eor.l	d7,d3

	move.l	d2,d6			; Swap 8x2, part 1
	move.l	d3,d7
	lsr.l	#8,d6
	lsr.l	#8,d7
	eor.l	d0,d6
	eor.l	d1,d7
	and.l	#$00ff00ff,d6
	and.l	#$00ff00ff,d7
	eor.l	d6,d0
	eor.l	d7,d1
	lsl.l	#8,d6
	lsl.l	#8,d7
	add.l	C2P1X1_8_C5_BM_040_DELTA4(sp),a1
	eor.l	d6,d2
	eor.l	d7,d3


	cmp.l	a0,a2				; End of line passed?
	bhs.s	.xstart

	add.w	C2P1X1_8_C5_BM_040_CHUNKYX(sp),a2 ; Skip to end of next
						  ; line
	add.l	C2P1X1_8_C5_BM_040_ROWMOD(sp),a1  ; Skip to beginning of
						  ; next line

.xstart

	move.l	d2,d6			; Swap 1x2, part 1
	move.l	d3,d7
	lsr.l	#1,d6
	lsr.l	#1,d7
	eor.l	d0,d6
	eor.l	d1,d7
	and.l	#$55555555,d6
	and.l	#$55555555,d7
	eor.l	d6,d0
	eor.l	d7,d1
	move.l	d0,(a1)
	add.l	d6,d6
	add.l	d7,d7
	eor.l	d6,d2
	eor.l	d7,d3

	move.l	a5,d6
	move.l	a6,d7
	move.l	d2,a3
	move.l	d3,a4

	move.l	d5,d2			; Swap 4x1, part 2
	move.l	d7,d3
	lsr.l	#4,d2
	lsr.l	#4,d3
	add.l	C2P1X1_8_C5_BM_040_DELTA5(sp),a1
	eor.l	d4,d2
	eor.l	d6,d3
	and.l	#$0f0f0f0f,d2
	and.l	#$0f0f0f0f,d3
	eor.l	d2,d4
	move.l	d1,(a1)
	eor.l	d3,d6

	lsl.l	#4,d2
	lsl.l	#4,d3
	eor.l	d2,d5
	eor.l	d3,d7

	move.l	d4,d2			; Swap 8x2, part 2
	move.l	d5,d3
	lsr.l	#8,d2
	lsr.l	#8,d3
	add.l	C2P1X1_8_C5_BM_040_DELTA6(sp),a1
	eor.l	d6,d2
	eor.l	d7,d3
	and.l	#$00ff00ff,d2
	and.l	#$00ff00ff,d3
	eor.l	d2,d6
	move.l	a3,(a1)
	eor.l	d3,d7

	lsl.l	#8,d2
	lsl.l	#8,d3
	eor.l	d2,d4
	eor.l	d3,d5

	move.l	d4,d2			; Swap 1x2, part 2
	move.l	d5,d3
	add.l	C2P1X1_8_C5_BM_040_DELTA7(sp),a1
	lsr.l	#1,d2
	lsr.l	#1,d3
	eor.l	d6,d2
	eor.l	d7,d3
	and.l	#$55555555,d2
	move.l	a4,(a1)
	and.l	#$55555555,d3

	eor.l	d2,d6
	eor.l	d3,d7
	add.l	d2,d2
	add.l	d3,d3
	add.l	C2P1X1_8_C5_BM_040_DELTA8(sp),a1
	eor.l	d2,d4
	eor.l	d3,d5

	move.l	d4,a3
	move.l	d5,a4

	cmp.l	a0,a2
	bne	.x

	subq.w	#1,C2P1X1_8_C5_BM_040_CHUNKYY(sp)
	bne	.y

	move.l	d6,(a1)
	add.l	C2P1X1_8_C5_BM_040_DELTA1(sp),a1
	move.l	d7,(a1)
	add.l	C2P1X1_8_C5_BM_040_DELTA2(sp),a1
	move.l	a3,(a1)
	add.l	C2P1X1_8_C5_BM_040_DELTA3(sp),a1
	move.l	a4,(a1)

.exit
	add.l	#C2P1X1_8_C5_BM_040_SIZEOF,sp
	movem.l	(sp)+,d2-d7/a2-a6
.earlyexit
	rts

