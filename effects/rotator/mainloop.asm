        xdef    _Rotator

        section ".text"

getuv   macro
        move.w  d0,d2
        move.w  d1,d3
        lsr.w   #7,d3
        move.b  d3,d2
        and.w   #$7ffe,d2
        endm

incuv   macro
        add.w   a3,d0
        add.w   a4,d1
        endm

; [a0] chunky
; [a1] txtHi
; [a2] txtLo
; [d3] du
; [d4] dv
; [d5] dU
; [d6] dV

WIDTH   equ 160
HEIGHT  equ 100

_Rotator:
        movem.w d2-d7/a3-a6,-(sp)

        move.w  d3,a3
        move.w  d4,a4
        move.w  d5,a5
        move.w  d6,a6

        clr.w   d0
        clr.w   d1

        move.w  #HEIGHT-1,d7
.loopy
        swap    d7
        movem.w d0-d1,-(sp)

        move.w  #WIDTH/8-1,d7
.loopx
        ; [a b c d e f g h] => [a b e f c d g h]

        getuv
        move.w  (a1,d2.w),d4
        incuv

        getuv
        or.w    (a2,d2.w),d4    ; [d4] hi0
        incuv

        getuv
        move.w  (a1,d2.w),d5
        incuv

        getuv
        or.w    (a2,d2.w),d5    ; [d5] lo0
        incuv

        getuv
        move.b  (a1,d2.w),d4
        incuv

        getuv
        or.b    (a2,d2.w),d4    ; [d4] (hi0 | hi1)
        incuv

        move.w  d4,(a0)+        ; *chunky++

        getuv
        move.b  (a1,d2.w),d5
        incuv

        getuv
        or.b    (a2,d2.w),d5    ; [d5] (lo0 | lo1)
        incuv

        move.w  d5,(a0)+        ; *chunky++

        dbf     d7,.loopx

        movem.w (sp)+,d0-d1
        add.w   a5,d0
        add.w   a6,d1
        swap    d7
        dbf     d7,.loopy

        movem.w (sp)+,d2-d7/a3-a6
        rts
