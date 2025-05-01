        include 'exec/types.i'

        xdef    _DrawTriPart

WIDTH           equ     128
WIDTH_BITS      equ     7

 STRUCTURE SIDE,0               ; Keep in sync with SideT from texobj.c !
        WORD    DY
        WORD    DXDY
        WORD    DUDY
        WORD    DVDY
        WORD    X
        WORD    U
        WORD    V
        LABEL   SIDE_SIZE

        section ".text"

; [a0] chunky
; [a1] texture (const)
; [a2] left (const)
; [a3] right (const)
; [d2] du (----UUuu)
; [d3] dv (----VVvv)
; [d6] ys
; [d7] ye
_DrawTriPart:
        movem.l d2-d7/a4,-(sp)

        ; [d7] height - must be positive
        sub.w   d6,d7
        subq.w  #1,d7
        bmi     .quit

        ; [a4] line = chunky + ys * WIDTH
        lsl.w   #WIDTH_BITS,d6
        lea     (a0,d6.w),a4

        ; keep xs and xe prepared for rounding
        add.w   #127,X(a2)
        add.w   #127,X(a3)

; in:
;  [d2] u (----UUuu)
;  [d3] v (----VVvv)
; out:
;  [d2] step (------UU)
;  [d3] step (uu--VVvv)

        swap    d3      ; VVvv----
        move.w  d2,d3   ; VVvvUUuu
        rol.w   #8,d3   ; VVvvuuUU
        clr.l   d2      ; --------
        move.b  d3,d2   ; ------UU
        clr.b   d3      ; VVvvuu--
        swap    d3      ; uu--VVvv

        ; [d5] u = l->u
        move.w  U(a2),d5

        ; [d6] v = l->v
        move.w  V(a2),d6

.loop:
        ; xs = (l->x + 127) >> 8
        clr.w   d4
        move.b  X(a2),d4

        ; xe = (r->x + 127) >> 8
        clr.w   d0
        move.b  X(a3),d0

        ; xs >= xe => skip
        cmp.w   d0,d4
        bge     .skip

        ; [a0] pixels = &line[xs]
        lea     (a4,d4.w),a0
        
        ; [d4] n = WIDTH - (xe - xs) = WIDTH + xs - xe
        add.w   #128,d4
        sub.w   d0,d4

        ; 12 is the size of single iteration
        move.w  d4,d0
        add.w   d0,d0
        add.w   d0,d0
        move.w  d0,d4
        add.w   d0,d0
        add.w   d0,d4
        ; same as `mulu.w #12,d4` but takes 24 cycles instead of 46

        ; [d0] u (per loop)
        move.w  d5,d0

        ; [d1] v (per loop)
        move.w  d6,d1

; in:
;  [d0] u (----UUuu)
;  [d1] v (----VVvv)
; out:
;  [d0] sum (------UU)
;  [d1] sum (uu--VVvv)

        swap    d1      ; VVvv----
        move.w  d0,d1   ; VVvvUUuu
        rol.w   #8,d1   ; VVvvuuUU
        clr.l   d0      ; --------
        move.b  d1,d0   ; ------UU
        clr.b   d1      ; VVvvuu--
        swap    d1      ; uu--VVvv
        
; jump into unrolled loop

        jmp     .start(pc,d4.w)

; Texturing code inspired by article by Kalms
; https://amycoders.org/opt/innerloops.html
.start:
        rept    WIDTH
        move.w  d1,d4           ;   [4] ----VVvv
        add.l   d3,d1           ;   [8] uu--VVvv 
        move.b  d0,d4           ;   [4] ----VVUU
        addx.b  d2,d0           ;   [4] ------UU
        move.b  (a1,d4.w),(a0)+ ;  [18]
        endr                    ; [=38]
        
.skip:  ; l->x += l->dxdy
        move.w  DXDY(a2),d4
        add.w   d4,X(a2)
        
        ; l->u += l->dudy
        add.w   DUDY(a2),d5

        ; l->v += l->dvdy
        add.w   DVDY(a2),d6

        ; r->x += r->dxdy
        move.w  DXDY(a3),d4
        add.w   d4,X(a3)
        
        ; line += WIDTH
        lea     WIDTH(a4),a4
        dbf     d7,.loop

        ; revert xs and xe to original form
        sub.w   #127,X(a2)
        sub.w   #127,X(a3)
        move.w  d5,U(a2)
        move.w  d6,V(a2)

.quit:  movem.l (sp)+,d2-d7/a4
        rts
