        section ".text"

group   macro                   ; cpu/bus cycles
        move.w  $1111(a1),\1    ; 12/3
        or.w    $2222(a2),\1    ; 12/3
        move.b  $3333(a1),\1    ; 12/3
        or.b    $4444(a2),\1    ; 12/3
        endm

        movem.w d2-d7,-(sp)

        group   d0
        group   d1
        group   d2
        group   d3
        group   d4
        group   d5
        group   d6
        group   d7              ; 48*8/12*8
        movem.w d0-d7,-(a0)     ; 8+4*8/8
                                ; 13.25/3.25 per pixel

        movem.w (sp)+,d2-d7
