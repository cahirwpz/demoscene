        xdef    _GenDrawSpan
        xdef    _Rotator

WIDTH   equ 160
HEIGHT  equ 100

        section ".text"

FOURPIX macro
        move.w  $1111(a1),\1
        or.w    $2222(a2),\1
        move.b  $3333(a1),\1
        or.b    $4444(a2),\1
        endm

GROUPSIZE equ 32

DrawSpan:
        rept    WIDTH/GROUPSIZE
        FOURPIX d0
        FOURPIX d1
        FOURPIX d2
        FOURPIX d3
        FOURPIX d4
        FOURPIX d5
        FOURPIX d6
        FOURPIX d7
        movem.w d0-d7,-(a0)
        endr

DrawSpanEnd:
        rts


GETUV   macro
        move.w	d1,d4   ; ----VVvv
	add.l	d3,d1
	move.b	d0,d4	; ----VVUU
	addx.b	d2,d0
        add.b   d4,d4
        and.w   #$7ffe,d4
        move.w  d4,\1
        endm

GROUPLEN equ (4+4*GROUPSIZE)

; [d2] du
; [d3] dv
_GenDrawSpan:
        movem.l d2-d4/d7,-(sp)

        lea     DrawSpanEnd(pc),a0

        clr.l   d0      ; ------UU 
        clr.l   d1      ; uu--VVvv

                        ; ----UUuu
                        ; ----VVvv
        lsl.l   #8,d2   ; --UUuu--
        swap    d3      ; VVvv----
        move.w  d2,d3   ; VVvvuu--
        swap    d3      ; uu--VVvv [d4]
        clr.w   d2      ; --UU----
        swap    d2      ; ------UU [d3]

        move.l	d3,d4   ; uu--VVvv
	clr.w	d4      ; uu------
	add.l	d4,d1   ; init X-flag

        move.w  #WIDTH/GROUPSIZE-1,d7
.loop32
        lea     -4(a0),a0
        swap    d7
        move.w  #4-1,d7
.loop8
        ; [a b c d e f g h] => [a b e f c d g h]
        lea     -32(a0),a0
        GETUV   30(a0)
        GETUV   26(a0)
        GETUV   14(a0)
        GETUV   10(a0)
        GETUV   22(a0)
        GETUV   18(a0)
        GETUV   6(a0)
        GETUV   2(a0)
        dbf     d7,.loop8

        swap    d7
        dbf     d7,.loop32

        movem.l (sp)+,d2-d4/d7
        rts

; [a0] chunky
; [a1] txtHi
; [a2] txtLo
; [d2] dU
; [d3] dV

_Rotator:
        movem.w d2-d7,-(sp)

        lea     WIDTH*HEIGHT/2(a0),a0
        clr.w   d0
        clr.w   d1

        move.w  #HEIGHT-1,d7
.loopy
        movem.l d0-d3/d7/a1-a2,-(sp)
        lsr.w   #8,d0
        move.b  d0,d1
        and.w   #$7ffe,d1
        add.w   d1,a1
        add.w   d1,a2
        bsr     DrawSpan
        movem.l (sp)+,d0-d3/d7/a1-a2
        add.w   d2,d0
        add.w   d3,d1
        dbf     d7,.loopy

        movem.w (sp)+,d2-d7
        rts
