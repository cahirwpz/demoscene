; vim: ft=asm68k:ts=8:sw=8:

        XDEF    _RenderFastUVMapOptimized
        XDEF    _RenderNormalUVMapOptimized
        XDEF    _UVMapComposeAndRenderOptimized

        section RenderNormalUVMapOptimized, code

; a0 [uint8_t *] mapU
; a1 [uint8_t *] mapV
; a2 [uint8_t *] texture
; a6 [uint8_t *] dst
; d5 [int] n
; d6 [int] offsetU
; d7 [int] offsetV

_RenderFastUVMapOptimized:
        movem.l d2-d7/a2-a6,-(sp)
        add.l   #32768,a2
        moveq.l #16,d4
        move.l  d5,a3
        move.b  d7,d6
        move.l  #$ff00ff00,d5
        move.l  #$00ff00ff,d7

.loop:
        move.l  (a0)+,d0        ; u1u2u3u4
        move.l  d0,d2

        move.l  (a1)+,d1        ; v1v2v3v4
        move.l  d1,d3

        and.l   d5,d0           ; u1--u3--
        and.l   d7,d2           ; --u2--u4

        and.l   d5,d1           ; v1--v3--
        and.l   d7,d3           ; --v2--v4

        lsr.l   #8,d1           ; --v1--v3
        lsl.l   #8,d2           ; u2--u4--

        or.l    d1,d0           ; u1v1u3v3
        or.l    d3,d2           ; u2v2u4v4

        move.l  d0,d1
        move.l  d2,d3

        lsr.l   d4,d1           ; ----u1v1
        lsr.l   d4,d3           ; ----u2v2

        add.w   d6,d0
        add.w   d6,d1
        add.w   d6,d2
        add.w   d6,d3

        move.b  (a2,d1.w),(a6)+
        move.b  (a2,d3.w),(a6)+
        move.b  (a2,d0.w),(a6)+
        move.b  (a2,d2.w),(a6)+

        subq.l  #4,a3
        tst.l   a3
        bgt.s   .loop

        movem.l (sp)+,d2-d7/a2-a6
        rts

; a0 [uint16_t *] mapU
; a1 [uint16_t *] mapV
; a2 [uint8_t *] texture
; a6 [uint8_t *] dst
; d5 [int] n
; d6 [int] offsetU
; d7 [int] offsetV

saved   equrl   d2-d7/a2-a6

_RenderNormalUVMapOptimized:
        movem.l saved,-(sp)
        add.l   #32768,a2
        move.b  d7,d6
        move.l  #$00ff00ff,d3
        moveq.l #16,d7
        nop

.loop:
        move.l  (a0)+,d0        ; ??uu??UU
        move.l  (a1)+,d1        ; ??vv??VV
        and.l   d3,d0           ; --vv--VV
        and.l   d3,d1           ; --uu--UU
        lsl.l   #8,d0           ; uu--UU--
        or.l    d1,d0           ; uuvvUUVV 
        move.l  d0,d2
        ror.l   d7,d2
        add.w   d6,d0
        add.w   d6,d2
        move.b  (a2,d2.w),(a6)+
        move.b  (a2,d0.w),(a6)+
        subq.l  #2,d5
        bgt.s   .loop

        movem.l (sp)+,saved
        rts

; a0 [uint8_t *] mapU
; a1 [uint8_t *] mapV
; a2 [uint8_t *] texture
; a3 [uint8_t *] cmap 
; a6 [uint8_t *] dst
; d4 [int] index
; d5 [int] n
; d6 [int] offsetU
; d7 [int] offsetV

_UVMapComposeAndRenderOptimized:
        movem.l d2-d7/a2-a6,-(sp)
        move.l  (a6),a0
        move.l  4(a6),a1
        move.l  8(a6),a2
        move.l  12(a6),a3
        move.l  16(a6),d5
        move.l  20(a6),d6
        move.l  24(a6),d7
        move.l  28(a6),a5
        move.l  32(a6),d4
        clr.l   d0
        move.b  d7,d6
        nop

.loop:
        cmp.b   (a5)+,d4
        bne.s   .skip

        move.b  (a0)+,d0
        lsl.w   #8,d0
        move.b  (a1)+,d0
        add.w   d6,d0
        move.b  (a2,d0.l),(a3)+
        bra.s   .cont

.skip:
        addq.l  #1,a0
        addq.l  #1,a1
        addq.l  #1,a3

.cont:
        subq.l  #1,d5
        bgt.s   .loop

        movem.l (sp)+,d2-d7/a2-a6
        rts
