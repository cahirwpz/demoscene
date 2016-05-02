; vim: ft=asm68k:ts=8:sw=8:

        include "exec/types.i"

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

        xdef    _UVMapComposeAndRenderOptimized

        section code

saved   equrl   d2-d7/a2-a4

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
