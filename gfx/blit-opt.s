; vim: ft=asm68k:ts=8:sw=8:

        XDEF    _PixBufBlitTransparent

        section BlitOptimized, code

; a0 [uint8_t *] dst
; a1 [uint8_t *] src
; d0 [int] width
; d1 [int] height
; d2 [int] sstride
; d3 [int] dstride

width   equr d0
height  equr d1
sstride equr a5
dstride equr a6

saved   equrl d2-d3/a5-a6

_PixBufBlitTransparent:
        movem.l saved,-(sp)
        move.l  d2,sstride
        move.l  d3,dstride

.y_loop:
        move.l  width,d3

.x_loop:
        move.b  (a1)+,d2
        beq.b   .zero
        move.b  d2,(a0)+
        subq.l  #1,d3
        bgt.s   .x_loop
        bra.s   .cont

.zero:
        addq.l  #1,a0
        subq.l  #1,d3
        bgt.s   .x_loop

.cont:
        add.l   dstride,a0
        add.l   sstride,a1

        subq.l  #1,height
        bgt.s   .y_loop

        movem.l (sp)+,saved
	rts
