        xdef    _GenDrawSpan
        xdef    _RenderRotator

WIDTH   equ 160
HEIGHT  equ 100
GROUPSIZE equ 32

        section ".text"

; Texturing code inspired by article by Kalms
; https://amycoders.org/opt/innerloops.html
GETUV   macro
        move.w  d1,d4   ; ----VVvv
        add.l   d3,d1
        move.b  d0,d4   ; ----VVUU
        addx.b  d2,d0
        and.w   d5,d4   ; [d5] $7ffe
        move.w  d4,\1
        endm

GROUPLEN equ (4+4*GROUPSIZE)

; [d2] du
; [d3] dv
_GenDrawSpan:
        movem.l d2-d7,-(sp)

        lea     DrawSpanEnd(pc),a0

        move.w  #$7ffe,d5

        clr.l   d0      ; ------UU 
        clr.l   d1      ; uu--VVvv

                        ; ----UUuu
                        ; ----VVvv
        lsl.l   #8,d2   ; --UUuu--
        add.l   d2,d2   ; lowest bit of UU is taken from fractional part!
        swap    d3      ; VVvv----
        move.w  d2,d3   ; VVvvuu--
        swap    d3      ; uu--VVvv [d4]
        clr.w   d2      ; --UU----
        swap    d2      ; ------UU [d3]

        move.l  d3,d4   ; uu--VVvv
        clr.w   d4      ; uu------
        add.l   d4,d1   ; init X-flag

        move.w  #WIDTH/GROUPSIZE-1,d7
.loop32
        lea     -132(a0),a0
        move.l  a0,a1
        swap    d7
        move.w  #4-1,d7
.loop8
        ; [a b c d e f g h] => [a b e f c d g h]
        GETUV   2(a1)
        GETUV   6(a1)
        GETUV   18(a1)
        GETUV   22(a1)
        GETUV   10(a1)
        GETUV   14(a1)
        GETUV   26(a1)
        GETUV   30(a1)
        lea     32(a1),a1
        dbf     d7,.loop8

        swap    d7
        dbf     d7,.loop32

        movem.l (sp)+,d2-d7
        rts

; [a0] chunky
; [a1] txtHi
; [a2] txtLo
; [d2] dU
; [d3] dV

_RenderRotator:
        movem.l d2-d7/a2-a4,-(sp)

        lea     WIDTH/2(a0),a0  ; the end of first line of chunky buffer
        swap    d0              ; store vital state in upper part...
        swap    d1              ; ...of data registers, for speed of course
        swap    d2
        swap    d3
        move.l  a1,a3           ; txtHi
        move.l  a2,a4           ; txtLo

        move.w  #HEIGHT-1,d7
LoopY:
        swap    d7
        move.l  d0,d4
        move.l  d1,d5
        swap    d4              ; tmpU
        swap    d5              ; tmpV
        lsr.w   #7,d4           ; tmpU >> 7
        move.b  d4,d5           ; (tmpV & 0xff00) | ((tmpU >> 7) & 0x00ff)
        and.w   #$7ffe,d5       ; offset must be even and limited to 32767
        lea     (a3,d5.w),a1
        lea     (a4,d5.w),a2

FOURPIX macro
        move.w  $1112(a1),\1
        or.w    $2222(a2),\1
        move.b  $3334(a1),\1
        or.b    $4444(a2),\1
        endm

        ; GenDrawSpan precalculates offsets for instructions in FOURPIX
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

        swap    d7
        lea     WIDTH(a0),a0    ; move to the end of next line
        add.l   d2,d0           ; lower part is always below 32768...
        add.l   d3,d1           ; ... so it will not influence upper part
        dbf     d7,LoopY

        movem.l (sp)+,d2-d7/a2-a4
        rts
