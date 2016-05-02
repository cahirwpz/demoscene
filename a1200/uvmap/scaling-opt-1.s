; vim: ft=asm68k:ts=8:sw=8:

        xdef    _StepperFromMap
        xdef    _FastStepperFromMap

        xref    _UVMapExpanderThreshold

        section code

; a0 [int *] map
; a1 [int *] stepper
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
        lea     (a0,d0.l*4),a2

        moveq.l #16,half
        move.l  _UVMapExpanderThreshold,d0
        asl.l   half,d0
        move.l  d0,max
        neg.l   d0
        move.l  d0,min
        move.l  #$01000000,wrap

.loop:
        move.l  (a2)+,d0
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
        move.l  d0,(a1)+
        subq.l  #1,d1
        bgt.s   .loop

        movem.l (sp)+,saved
        rts


; a0 [int *] map
; a1 [int *] stepper
; d0 [int] width
; d1 [int] height

_FastStepperFromMap:
        movem.l saved,-(sp)
        move.l  d1,d4
        subq.l  #1,d4
        mulu.l  d0,d4
        lea     (a0,d0.l*4),a2

.loop:
        move.l  (a2)+,d0
        move.l  (a2)+,d1
        move.l  (a2)+,d2
        move.l  (a2)+,d3
        sub.l   (a0)+,d0
        sub.l   (a0)+,d1
        sub.l   (a0)+,d2
        sub.l   (a0)+,d3
        asr.l   #3,d0
        asr.l   #3,d1
        asr.l   #3,d2
        asr.l   #3,d3
        move.l  d0,(a1)+
        move.l  d1,(a1)+
        move.l  d2,(a1)+
        move.l  d3,(a1)+
        subq.l  #4,d4
        bgt.s   .loop

        movem.l (sp)+,saved
        rts
