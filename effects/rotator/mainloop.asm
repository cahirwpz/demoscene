        xdef    _Rotator

        section ".text"

; [a0] chunky
; [a1] txtHi
; [a2] txtLo
; [d3] du
; [d4] dv
; [d5] dU
; [d6] dV

WIDTH   equ 160
HEIGHT  equ 100

getuv   macro
        move.w	d1,d2   ; ----VVvv
	add.l	d4,d1
	move.b	d0,d2	; ----VVUU
	addx.b	d3,d0
        add.b   d2,d2
        and.w   #$7ffe,d2
        endm

_Rotator:
        movem.w d2-d7/a3-a6,-(sp)

        clr.l   d0      ; ------UU 
        clr.l   d1      ; uu--VVvv

                        ; ----UUuu
                        ; ----VVvv
        lsl.l   #8,d3   ; --UUuu--
        swap    d4      ; VVvv----
        move.w  d3,d4   ; VVvvuu--
        swap    d4      ; uu--VVvv [d4]
        clr.w   d3      ; --UU----
        swap    d3      ; ------UU [d3]

        move.w  d5,a5
        move.w  d6,a6

        move.w  #HEIGHT-1,d7
.loopy
        swap    d7
        movem.l d0-d1,-(sp)

                        ; ----UUuu
                        ; ----VVvv
        lsl.l   #8,d0   ; --UUuu--
        swap    d1      ; VVvv----
        move.w  d0,d1   ; VVvvuu--
        swap    d1      ; uu--VVvv [d1]
        clr.w   d0      ; --UU----
        swap    d0      ; ------UU [d0]

        move.l	d4,d2   ; uu--VVvv
	clr.w	d2      ; uu------
	add.l	d2,d1   ; init X-flag

        move.w  #WIDTH/8-1,d7
.loopx
        ; [a b c d e f g h] => [a b e f c d g h]

        getuv
        move.w  (a1,d2.w),d5

        getuv
        or.w    (a2,d2.w),d5    ; [d5] hi0

        getuv
        move.w  (a1,d2.w),d6

        getuv
        or.w    (a2,d2.w),d6    ; [d6] lo0

        getuv
        move.b  (a1,d2.w),d5

        getuv
        or.b    (a2,d2.w),d5    ; [d5] (hi0 | hi1)
        move.w  d5,(a0)+        ; *chunky++

        getuv
        move.b  (a1,d2.w),d6

        getuv
        or.b    (a2,d2.w),d6    ; [d6] (lo0 | lo1)
        move.w  d6,(a0)+        ; *chunky++

        dbf     d7,.loopx

        movem.l (sp)+,d0-d1
        add.w   a5,d0
        add.w   a6,d1
        swap    d7
        dbf     d7,.loopy

        movem.w (sp)+,d2-d7/a3-a6
        rts
