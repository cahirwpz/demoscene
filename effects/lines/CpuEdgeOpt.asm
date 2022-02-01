        xdef    _CpuEdgeOpt

        section ".text"

; [a0] bitplane
; [a1] stride
; [d0] xs
; [d1] ys
; [d2] xe
; [d3] ye

_CpuEdgeOpt:
        movem.l d2-d5,-(sp)

        cmp.w   d3,d1           ; ys > ye ?
        ble.s   .y_ok
        exg.l   d0,d2           ; xs <=> xe
        exg.l   d1,d3           ; ys <=> ye

.y_ok   sub.w   d0,d2           ; [d2] dx = xe - xs
        sub.w   d1,d3           ; [d3] dy = ye - ys
        beq.s	exit

        move.w  a1,d4
        muls.w  d1,d4
        add.l   d4,a0           ; pixels += ys * stride

        move.w  d0,d5
        lsr.w   #3,d5
        and.w   #-2,d5
        add.w   d5,a0           ; pixels += (xs >> 3) & -2

        move.w  d2,d4           ; [d4] abs(dx)
        bge.s   .dx_ok
        neg.w   d4

.dx_ok  divs.w  d3,d4           ; [d4] di
        move.l  d4,d5
        swap    d5              ; [d5] df

        clr.w   d0
        clr.w   d1

        tst.w   d2
        blt.s   case2

case1:

.loop
        dbf     d2,.loop
        bra.s   exit

case2:

.loop
        dbf     d2,.loop

exit:   movem.l (sp)+,d2-d5
        rts
