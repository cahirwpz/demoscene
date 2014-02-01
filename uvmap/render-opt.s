; vim: ft=asm68k:ts=8:sw=8:

        INCLUDE "exec/types.i"

   STRUCTURE    UVMapRenderer,0
        LONG    mapSize
        APTR    mapU
        APTR    mapV
        APTR    texture
        APTR    pixmap
        APTR    colorMap
        APTR    lightMap
        UWORD   offset
        UBYTE   colorIndex
	LABEL   UVMapRenderer_SIZE

        XDEF    _RenderFastUVMapOptimized
        XDEF    _RenderFastUVMapWithLightOptimized
        XDEF    _RenderNormalUVMapOptimized
        XDEF    _UVMapComposeAndRenderOptimized

        SECTION RenderNormalUVMapOptimized,CODE

; a6 [UVMapRendererT *] renderer

saved   EQURL   d2-d4/d6-d7/a2-a3

_RenderFastUVMapOptimized:
        movem.l saved,-(sp)
        move.l  mapU(a6),a0
        move.l  mapV(a6),a1
        move.l  texture(a6),a2
        move.l  pixmap(a6),a3
        move.w  offset(a6),d6
        move.l  mapSize(a6),d7
        clr.l   d0
        clr.l   d1
        nop

.loop:  move.w  (a0)+,d0        ; u1u2
        move.w  d0,d1           ; u1u2

        move.w  (a1)+,d2        ; v1v2
        move.w  d2,d3           ; v1v2

        lsl.w   #8,d1           ; u2--
        lsr.w   #8,d3           ; --v1

        move.b  d3,d0           ; u1v1
        move.b  d2,d1           ; u2v2

        add.w   d6,d0
        add.w   d6,d1

        move.w  (a2,d0.l),d0
        move.b  (a2,d1.l),d0
        move.w  d0,(a3)+

        subq.l  #2,d7
        bgt     .loop

        movem.l (sp)+,saved
        rts

_RenderFastUVMapWithLightOptimized:
        movem.l saved,-(sp)
        move.l  mapU(a6),a0
        move.l  mapV(a6),a1
        move.l  texture(a6),a2
        move.l  pixmap(a6),a3
        move.l  lightMap(a6),a4
        move.l  colorMap(a6),a5
        move.w  offset(a6),d6
        move.l  mapSize(a6),d7
        add.l   #32768,a2
        clr.l   d4
        clr.l   d5
        nop

.loop:  move.w  (a0)+,d0        ; u1u2
        move.w  d0,d1           ; u1u2

        move.w  (a1)+,d2        ; v1v2
        move.w  d2,d3           ; v1v2

        lsl.w   #8,d1           ; u2--
        lsr.w   #8,d3           ; --v1

        move.b  d3,d0           ; u1v1
        move.b  d2,d1           ; u2v2

        add.w   d6,d0
        add.w   d6,d1

        move.w  (a2,d0.w),d4
        move.w  (a2,d1.w),d5

        move.b  (a4)+,d4
        move.b  (a4)+,d5

        move.w  (a5,d4.l),d0
        move.b  (a5,d5.l),d0
        
        move.w  d0,(a3)+

        subq.l  #2,d7
        bgt     .loop

        movem.l (sp)+,saved
        rts

; a6 [UVMapRendererT *] renderer

saved   EQURL   d2-d4/d6-d7/a2-a3

_RenderNormalUVMapOptimized:
        movem.l saved,-(sp)
        move.l  mapU(a6),a0
        move.l  mapV(a6),a1
        move.l  texture(a6),a2
        move.l  pixmap(a6),a3
        move.w  offset(a6),d6
        move.l  mapSize(a6),d7
        add.l   #32768,a2
        move.l  #$00ff00ff,d3
        moveq.l #16,d4
        nop

.loop:  move.l  d3,d0
        move.l  d3,d1

        move.l  (a0)+,d2        ; ??u1??u2
        and.l   d2,d0           ; --u1--u2

        move.l  (a1)+,d2        ; ??v1??v2
        and.l   d2,d1           ; --v1--v2

        ror.l   #8,d0           ; u2--u1--
        ror.l   d4,d1           ; --v2--v1

        or.l    d1,d0           ; u2v2u1v1
        move.l  d0,d1           ; u2v2u1v1

        add.w   d6,d0           ; u1v1 + offset
        lsr.l   d4,d1           ; u2v2

        add.w   d6,d1           ; u2v2 + offset
        move.w  (a2,d0.w),d2
        move.b  (a2,d1.w),d2

        move.w  d2,(a3)+

        subq.l  #2,d7
        bgt     .loop

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

saved   EQURL   d2-d7/a2-a4

_UVMapComposeAndRenderOptimized:
        movem.l saved,-(sp)
        move.l  mapU(a6),a0
        move.l  mapV(a6),a1
        move.l  texture(a6),a2
        move.l  pixmap(a6),a3
        move.l  colorMap(a6),a4
        move.b  colorIndex(a6),d4
        move.w  offset(a6),d6
        move.l  mapSize(a6),d7
        clr.l   d0
        nop

.loop:
        cmp.b   (a4)+,d4
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
        subq.l  #1,d7
        bgt.s   .loop

        movem.l (sp)+,saved
        rts
