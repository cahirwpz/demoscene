; vim: ft=asm68k:ts=8:sw=8:

        xdef    _CpuLineOpt

        section ".text"

; [a0] bitplane
; [a1] stride
; [d0] xs
; [d1] ys
; [d2] xe
; [d3] ye

EDGE	equ	0

_CpuLineOpt:
        movem.l d2-d5,-(sp)

        cmp.w   d3,d1           ; ys > ye ?
        ble.s   .y_ok
        exg.l   d0,d2           ; xs <=> xe
        exg.l   d1,d3           ; ys <=> ye

.y_ok   move.w  a1,d4
        muls.w  d1,d4
        add.l   d4,a0           ; pixels += ys * stride

        move.w  d0,d5
        lsr.w   #3,d5
        and.w   #-2,d5
        add.w   d5,a0           ; pixels += (xs >> 3) & -2

        move.w  d0,d5
	not.w	d5
        andi.w  #15,d5
	moveq	#0,d4
	bset.l	d5,d4           ; [d4] color = 0x8000 >> (xs & 15)

        sub.w   d0,d2           ; [d2] dx = abs(xe - xs)
        move.w  d2,d0           ; [d0] xe - xs
        bge.s   .dx_ok
        neg.w   d2

.dx_ok  sub.w   d1,d3           ; [d3] dy = ye - ys

        cmp.w   d3,d2           ; dx < dy ?
        bge.s   dy_dx

; dx < dy

dx_dy:  tst.w   d3              ; dy == 0 ?
        beq	exit

        add.w   d2,d2           ; [d2] dg2 = 2 * dx
        move.w  d2,d1
        sub.w   d3,d1           ; [d1] dg = 2 * dx - dy
        move.w  d3,d5
        add.w   d3,d5           ; [d5] dg1 = 2 * dy

        tst.w   d0              ; xe < xs ?
        bge.s   case2

case1:  sub.w   d2,d1           ; precompensate for [dg += dg2]

.loop
	ifeq EDGE
	or.w    d4,(a0)         ; [12] *pixels |= color
	else
	eor.w	d4,(a0)
	endif

        add.w   d2,d1           ; [4] dg += dg2
        blt.s   .skip           ; [8/10] dg > 0

	sub.w   d5,d1           ; [4] dg -= dg1

        rol.w   #1,d4		; [4]
        bcs.s   .skip2		; [8/10]

.skip	add.w   a1,a0           ; [8] pixels += stride
	dbf     d3,.loop	; [10]
	bra.s   exit		; ~68 cycles per pixel

.skip2	lea	-2(a0,a1.w),a0
	dbf     d3,.loop	; [10]
	bra.s   exit		; ~68 cycles per pixel

case2:  sub.w   d2,d1           ; precompensate for [dg += dg2]

.loop
	ifeq EDGE
	or.w    d4,(a0)         ; *pixels |= color
	else
	eor.w   d4,(a0)
	endif

        add.w   d2,d1           ; dg += dg1
        blt.s   .skip           ; dg > 0

	sub.w   d5,d1           ; dg -= dg2

        ror.w   #1,d4
        bcs.s   .skip2

.skip	add.w   a1,a0           ; pixels += stride
	dbf     d3,.loop
        bra.s   exit

.skip2	lea	2(a0,a1.w),a0
	dbf     d3,.loop
        bra.s   exit

; dx >= dy

dy_dx:  tst.w   d2              ; dx == 0 ?
        beq.s   exit

        add.w   d3,d3           ; [d3] dg2 = 2 * dy
        move.w  d3,d1
        sub.w   d2,d1           ; [d1] dg = 2 * dy - dx
        move.w  d2,d5
        add.w   d2,d5           ; [d5] dg1 = 2 * dx

        tst.w   d0              ; xe < xs ?
        bge.s   case4

case3:  sub.w   d3,d1           ; precompensate for [dg += dg2]

.loop
	ifeq	EDGE
	or.w    d4,(a0)         ; *pixels |= color
	endif

	add.w   d3,d1           ; dg += dg2
        blt.s   .else           ; dg > 0

        sub.w   d5,d1           ; dg -= dg1
	ifne	EDGE
	eor.w	d4,(a0)
	endif
        add.w   a1,a0           ; pixels += stride

.else	rol.w   #1,d4
        bcs.s   .skip
        dbf     d2,.loop
        bra.s   exit

.skip   subq.l  #2,a0
	dbf     d2,.loop
        bra.s   exit

case4:  sub.w   d3,d1           ; precompensate for [dg += dg2]

.loop   
	ifeq	EDGE
	or.w    d4,(a0)         ; *pixels |= color
	endif

	add.w   d3,d1           ; dg += dg2
        blt.s   .else           ; dg > 0

        sub.w   d5,d1           ; dg -= dg1
	ifne	EDGE
	eor.w	d4,(a0)
	endif
        add.w   a1,a0           ; pixels += stride

.else	ror.w   #1,d4
        bcs.s   .skip
	dbf     d2,.loop
	bra.s	exit

.skip   addq.l  #2,a0
	dbf     d2,.loop

exit:   movem.l (sp)+,d2-d5
        rts
