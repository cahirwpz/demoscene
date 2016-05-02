; vim: ft=asm68k:ts=8:sw=8:

        xdef    _RawAddAndClamp
        xdef    _RawSubAndClamp

        section code

saved   equrl   d2-d3

; a0 [uint8_t *] dst
; a1 [uint8_t *] src
; d0 [int] size
; d1 [uint8_t] value

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

