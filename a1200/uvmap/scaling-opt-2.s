; vim: ft=asm68k:ts=8:sw=8:

        xdef    _FastExpandLine8x
        xdef    _ExpandLine8x
        xdef    _Increment

        xref    _UVMapExpanderThreshold

        section code

; a0 [int16_t *] dst
; a1 [int *] src
; d2 [int] width

half    equr    d2
dx      equr    d3
xs      equr    d4
xe      equr    d5
wrap    equr    d7
max     equr    a2
min     equr    a3
saved   equrl   d2-d7/a2-a3

_ExpandLine8x:
        movem.l saved,-(sp)

        move.l  (a1)+,xs
        move.l  (a1)+,xe
        moveq.l #16,half
        move.l  _UVMapExpanderThreshold,d0
        asl.l   half,d0
        move.l  d0,max
        neg.l   d0
        move.l  d0,min
        move.l  #$01000000,wrap

        nop

.loop:
        move.l  xe,dx
        sub.l   xs,dx

.plus:
        cmp.l   max,dx
        ble.s   .minus
        
        sub.l   wrap,dx
        bra.s   .ok

.minus:  
        cmp.l   min,dx
        bge.s   .ok

        add.l   wrap,dx
        bra.s   .ok

.ok:
        move.l  xs,d0
        asr.l   #3,dx

        move.l  d0,d1
        lsr.l   half,d0
        move.w  d0,(a0)+
        add.l   dx,d1

        move.l  d1,d0
        lsr.l   half,d1
        move.w  d1,(a0)+
        add.l   dx,d0

        move.l  d0,d1
        lsr.l   half,d0
        move.w  d0,(a0)+
        add.l   dx,d1

        move.l  d1,d0
        lsr.l   half,d1
        move.w  d1,(a0)+
        add.l   dx,d0

        move.l  d0,d1
        lsr.l   half,d0
        move.w  d0,(a0)+
        add.l   dx,d1

        move.l  d1,d0
        lsr.l   half,d1
        move.w  d1,(a0)+
        add.l   dx,d0

        move.l  d0,d1
        lsr.l   half,d0
        move.w  d0,(a0)+
        add.l   dx,d1

        move.l  d1,d0
        lsr.l   half,d1
        move.w  d1,(a0)+
        add.l   dx,d0

        move.l  xe,xs
        move.l  (a1)+,xe

        subq.l  #1,d2
        bne.s   .loop

        movem.l (sp)+,saved
        rts

; a0 [int16_t *] dst
; a1 [int *] src
; d0 [int] width

_FastExpandLine8x:
        movem.l saved,-(sp)

        move.l  (a1)+,xs
        move.l  (a1)+,xe
        moveq.l #16,half

.loop:
        move.l  xe,dx
        sub.l   xs,dx
        move.l  xs,d6
        asr.l   #3,dx

        rept    8
        move.l  d6,d1
        lsr.l   half,d6
        move.w  d6,(a0)+
        add.l   dx,d1
        endr

        move.l  xe,xs
        move.l  (a1)+,xe

        subq.l  #1,d0
        bne.s   .loop

        movem.l (sp)+,saved
        rts

; a0 [int *] x
; a1 [int *] dx
; d1 [int] width

_Increment:
.loop   move.l  (a1)+,d0
        add.l   d0,(a0)+
        subq.l  #1,d1
        bne.s   .loop
        rts
