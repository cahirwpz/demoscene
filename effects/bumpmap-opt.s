; vim: ft=asm68k:ts=8:sw=8:

        XDEF    _RenderBumpMapOptimized

        SECTION BumpMapOptimized,code

; a0 [int16_t *] mapU
; a1 [int16_t *] mapV
; a2 [uint8_t *] rmap
; a3 [uint8_t *] dst
; d0 [int16_t] width
; d1 [int16_t] height
; d2 [int16_t] light_x
; d3 [int16_t] light_y

saved   EQURL d2-d7/a2-a3

_RenderBumpMapOptimized:
        movem.l saved,-(sp)

        lea     2(a0,d0.w*2),a0 ; mapU += width + 1
        lea     2(a1,d0.w*2),a1 ; mapV += width + 1
        lea     1(a3,d0.w),a3   ; dst  += width + 1

        sub.w   #1,d0           ; d0 = width - 1
        sub.w   #1,d1           ; d1 = height - 1

        moveq   #1,d4
        moveq   #1,d5
        sub.w   d2,d4           ; d4 = 1 - light_x
        sub.w   d3,d5           ; d5 = 1 - light_y

        sub.w   d2,d0           ; d0 = width - 1 - light_x 
        sub.w   d3,d1           ; d1 = height - 1 - light_y

        clr.l   d6
        move.w  #256,d7

.loop_y:
        move.w  d4,-(sp)

.loop_x:
        move.w  (a0)+,d2
        move.w  (a1)+,d3

        add.w   d4,d2
        add.w   d5,d3

        cmp.w   d7,d3
        scs     d6
        and.l   d6,d3           ; set to 0 if not in [0, 255]
        lsl.w   #8,d3

        cmp.w   d7,d2
        scs     d6
        and.l   d6,d2           ; set to 0 if not in [0, 255]
        move.b  d2,d3

        add.w   #1,d4

        move.b  (a2,d3.l),(a3)+

        cmp.w   d0,d4
        blt     .loop_x

        move.w  (sp)+,d4

        add.l   #4,a0
        add.l   #4,a1
        add.l   #2,a3

        add.w   #1,d5
        cmp.w   d1,d5
        blt     .loop_y

        movem.l (sp)+,saved
        rts
