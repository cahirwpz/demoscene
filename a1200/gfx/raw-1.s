; vim: ft=asm68k:ts=8:sw=8:

        xdef    _RawBlitNormal
        xdef    _RawBlitTransparent
        xdef    _RawBlitColorMap
        xdef    _RawBlitColorFunc

        section code

width   equr    d0
height  equr    d1
dstride equr    a5
sstride equr    a6
saved   equrl   d2-d3/a5-a6

; a0 [uint8_t *] dst
; a1 [uint8_t *] src
; d0 [int] width
; d1 [int] height
; d2 [int] sstride
; d3 [int] dstride

_RawBlitNormal:
        movem.l saved,-(sp)
        subq.l  #1,height
        subq.l  #1,width
        move.l  d2,sstride
        move.l  d3,dstride

.y_loop:
        move.l  width,d3

.x_loop:
        move.b  (a1)+,(a0)+
        dbra    d3,.x_loop

        add.l   dstride,a0
        add.l   sstride,a1
        dbra    height,.y_loop

        movem.l (sp)+,saved
	rts

; a0 [uint8_t *] dst
; a1 [uint8_t *] src
; d0 [int] width
; d1 [int] height
; d2 [int] sstride
; d3 [int] dstride

_RawBlitTransparent:
        movem.l saved,-(sp)
        subq.l  #1,height
        subq.l  #1,width
        move.l  d2,sstride
        move.l  d3,dstride

.y_loop:
        move.l  width,d3

.x_loop:
        move.b  (a1)+,d2
        beq.b   .zero
        move.b  d2,(a0)+
        dbra    d3,.x_loop
        bra.s   .cont

.zero:
        addq.l  #1,a0
        dbra    d3,.x_loop

.cont:
        add.l   dstride,a0
        add.l   sstride,a1
        dbra    height,.y_loop

        movem.l (sp)+,saved
	rts

; a0 [uint8_t *] dst
; a1 [uint8_t *] src
; a2 [uint8_t *] cmap
; d0 [int] width
; d1 [int] height
; d2 [int] sstride
; d3 [int] dstride

_RawBlitColorMap:
        movem.l saved,-(sp)
        subq.l  #1,height
        subq.l  #1,width
        move.l  d2,sstride
        move.l  d3,dstride

.y_loop:
        move.l  width,d2

.x_loop:
        move.w  (a0),d3
        move.b  (a1)+,d3
        move.b  (a2,d3.l),(a0)+
        dbra    d2,.x_loop

.cont:
        add.l   dstride,a0
        add.l   sstride,a1
        dbra    height,.y_loop

        movem.l (sp)+,saved
	rts

; a0 [uint8_t *] dst
; a1 [uint8_t *] src
; a2 [uint8_t *] cfunc
; d0 [int] width
; d1 [int] height
; d2 [int] sstride
; d3 [int] dstride

_RawBlitColorFunc:
        movem.l saved,-(sp)
        subq.l  #1,height
        subq.l  #1,width
        move.l  d2,sstride
        move.l  d3,dstride
        clr.l   d3

.y_loop:
        move.l  width,d2

.x_loop:
        move.b  (a1)+,d3
        move.b  (a2,d3.w),(a0)+
        dbra    d2,.x_loop

.cont:
        add.l   dstride,a0
        add.l   sstride,a1
        dbra    height,.y_loop

        movem.l (sp)+,saved
	rts
