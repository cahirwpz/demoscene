; vim: ft=asm68k:ts=8:sw=8:

        XDEF    _StepperFromMap
        XDEF    _FastStepperFromMap
        XDEF    _FastExpandLine8x
        XDEF    _ExpandLine8x
        XDEF    _Increment

        XREF    _UVMapExpanderThreshold

        section ScalingOptimized, code

; a0 [int *] map
; a2 [int *] stepper
; d0 [int] width
; d1 [int] height

half    equr    d2
min     equr    d3
max     equr    d4
wrap    equr    d5
saved   equrl   d2-d5/a2

_StepperFromMap:
        movem.l saved,-(sp)
        subq.l  #1,d1
        mulu.l  d0,d1
        lea     (a0,d0.l*4),a1

        moveq.l #16,half
        move.l  _UVMapExpanderThreshold,d0
        asl.l   half,d0
        move.l  d0,max
        neg.l   d0
        move.l  d0,min
        move.l  #$01000000,wrap

.loop:
        move.l  (a1)+,d0
        sub.l   (a0)+,d0

        cmp.l   max,d0
        ble.s   .minus
        
        sub.l   wrap,d0
        bra.s   .ok

.minus:  
        cmp.l   min,d0
        bge.s   .ok

        add.l   wrap,d0
        bra.s   .ok

.ok:
        asr.l   #3,d0
        move.l  d0,(a2)+
        subq.l  #1,d1
        bgt.s   .loop

        movem.l (sp)+,saved
        rts


; a0 [int *] map
; a2 [int *] stepper
; d0 [int] width
; d1 [int] height

saved   equrl   d2-d4/a2

_FastStepperFromMap:
        movem.l saved,-(sp)
        move.l  d1,d4
        subq.l  #1,d4
        mulu.l  d0,d4
        lea     (a0,d0.l*4),a1

.loop:
        move.l  (a1)+,d0
        move.l  (a1)+,d1
        move.l  (a1)+,d2
        move.l  (a1)+,d3
        sub.l   (a0)+,d0
        sub.l   (a0)+,d1
        sub.l   (a0)+,d2
        sub.l   (a0)+,d3
        asr.l   #3,d0
        asr.l   #3,d1
        asr.l   #3,d2
        asr.l   #3,d3
        move.l  d0,(a2)+
        move.l  d1,(a2)+
        move.l  d2,(a2)+
        move.l  d3,(a2)+
        subq.l  #4,d4
        bgt.s   .loop

        movem.l (sp)+,saved
        rts

; a0 [int16_t *] dst
; a1 [int *] src
; d2 [int] width

half    equr    d3
dx      equr    d4
xs      equr    d5
xe      equr    d6
saved   equrl   d2-d6

_FastExpandLine8x:
        movem.l saved,-(sp)

        move.l  (a1)+,xs
        move.l  (a1)+,xe
        moveq.l #16,half

.loop:
        move.l  xe,dx
        sub.l   xs,dx
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
; d2 [int] width

saved   equrl   d2-d7/a2-a3
wrap    equr    d7
max     equr    a2
min     equr    a3

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

; a0 [int *] x
; a1 [int *] dx
; d1 [int] width

_Increment:
.loop   move.l  (a1)+,d0
        add.l   d0,(a0)+
        subq.l  #1,d1
        bne.s   .loop
        rts
