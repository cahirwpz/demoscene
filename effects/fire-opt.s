; vim: ft=asm68k:ts=8:sw=8:

        SECTION FireOptimizedCode,code

        XDEF    _InitFireTables
        XDEF    _CalculateFire
        XDEF    _FireMainTable
        XDEF    _FireBorderTable

_InitFireTables:
        lea     _FireMainTable,a0
        lea     _FireBorderTable,a1

        clr.b   d1

.loop:
        move.b  d1,d0 
        beq     .zero
        sub.b   #1,d0

.zero:
        REPT    4
        move.b  d0,(a0)+
        ENDR
        REPT    3
        move.b  d0,(a1)+
        ENDR
        add.b   #1,d1
        bcc     .loop
        rts

; a0 [uint8_t *] last line
; d0 [uint16_t] width
; d1 [uint16_t] height

saved   EQURL   d2-d7/a2-a5

_CalculateFire:
        movem.l saved,-(sp)

        lea     _FireMainTable,a4
        lea     _FireBorderTable,a5
        move.w  d0,d7
        neg.w   d7
        subq.w  #3,d0
        subq.w  #1,d1
        clr.l   d2
        clr.l   d3
        clr.l   d4
        clr.l   d5

.y_loop:
        move.l  a0,-(sp)                ; a0 = left
        move.l  a0,a1                   ; a1 = center
        lea     1(a0),a2                ; a2 = right
        lea     (a0,d7.w),a3            ; a3 = upper

        move.b  (a1)+,d2
        move.b  (a2)+,d3
        move.b  (a3),d4
        move.w  d2,d6
        add.w   d3,d6
        add.w   d4,d6
        move.b  (a5,d6.w),(a3)+         ; *upper++ = FireBorderTable[c + r + u]

        move.w  d0,-(sp)                ; d0 = n

.x_loop:
        move.b  (a0)+,d2
        move.b  (a1)+,d3
        move.b  (a2)+,d4
        move.b  (a3),d5
        move.w  d2,d6
        add.w   d3,d6
        add.w   d4,d6
        add.w   d5,d6
        move.b  (a4,d6.w),(a3)+         ; *upper++ = FireMainTable[l + c + r + u]
        dbf     d0,.x_loop

        move.w  (sp)+,d0

        move.b  (a0),d2
        move.b  (a1),d3
        move.b  (a3),d4
        move.w  d2,d6
        add.w   d3,d6
        add.w   d4,d6
        move.b  (a5,d6.w),(a3)          ; *upper++ = FireBorderTable[l + c + u]

        move.l  (sp)+,a0
        
        lea     (a0,d7.w),a0            ; move to the upper line

        dbf     d1,.y_loop

        movem.l (sp)+,saved
        rts
