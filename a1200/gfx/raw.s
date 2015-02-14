; vim: ft=asm68k:ts=8:sw=8:

        XDEF    _RawBlitNormal
        XDEF    _RawBlitTransparent
        XDEF    _RawBlitColorMap
        XDEF    _RawBlitColorFunc
        XDEF    _RawAddAndClamp
        XDEF    _RawSubAndClamp

        section GfxRaw, code

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

width   equr d0
height  equr d1
sstride equr a5
dstride equr a6

saved   equrl d2-d3/a5-a6

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

saved   equrl d2-d3/a5-a6

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

saved   equrl d2-d3/a5-a6

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

; a0 [uint8_t *] dst
; a1 [uint8_t *] src
; d0 [int] size
; d1 [uint8_t] value

saved   equrl   d2-d3

_RawAddAndClamp:
        movem.l saved,-(sp)
        clr.l   d3

.loop: 
        move.l  (a1)+,d2        ; abcd
        add.b   d1,d2
        scs.b   d3              ; d-mask
        ror.l   #8,d2
        ror.l   #8,d3
        add.b   d1,d2
        scs.b   d3              ; c-mask
        ror.l   #8,d2
        ror.l   #8,d3
        add.b   d1,d2
        scs.b   d3              ; b-mask
        ror.l   #8,d2
        ror.l   #8,d3
        add.b   d1,d2
        scs.b   d3              ; a-mask
        ror.l   #8,d2
        ror.l   #8,d3
        or.l    d3,d2
        move.l  d2,(a0)+

        subq.l  #4,d0
        bgt.s   .loop

        movem.l (sp)+,saved
        rts

; a0 [uint8_t *] dst
; a1 [uint8_t *] src
; d0 [int] size
; d1 [uint8_t] value

saved   equrl   d2-d3

_RawSubAndClamp:
        movem.l saved,-(sp)
        clr.l   d3

.loop: 
        move.l  (a1)+,d2        ; abcd
        sub.b   d1,d2
        scc.b   d3              ; d-mask
        ror.l   #8,d2
        ror.l   #8,d3
        sub.b   d1,d2
        scc.b   d3              ; c-mask
        ror.l   #8,d2
        ror.l   #8,d3
        sub.b   d1,d2
        scc.b   d3              ; b-mask
        ror.l   #8,d2
        ror.l   #8,d3
        sub.b   d1,d2
        scc.b   d3              ; a-mask
        ror.l   #8,d2
        ror.l   #8,d3
        and.l   d3,d2
        move.l  d2,(a0)+

        subq.l  #4,d0
        bgt.s   .loop

        movem.l (sp)+,saved
        rts
