; vim: ft=asm68k:ts=8:sw=8:

        xdef    _CpuEdgeOpt

        section ".text"

; [a0] bitplane
; [a1] stride
; [d0] xs
; [d1] ys
; [d2] xe
; [d3] ye

_CpuEdgeOpt:
        movem.l d2-d6,-(sp)

        cmp.w   d3,d1           ; ys > ye ?
        ble.s   .y_ok
        exg.l   d0,d2           ; xs <=> xe
        exg.l   d1,d3           ; ys <=> ye

.y_ok   sub.w   d0,d2           ; [d2] dx = xe - xs
        sub.w   d1,d3           ; [d3] dy = ye - ys
        beq.s	.exit

        move.w  a1,d4
        muls.w  d1,d4
        add.l   d4,a0           ; pixels += ys * stride

        move.w  d2,d4           ; [d4] abs(dx)
        bge.s   .dx_ok
        neg.w   d4

.dx_ok  divs.w  d3,d4           ; [d4] (df << 16) | di

        swap    d0
        clr.w   d0              ; [d0] x = (xi << 16) | xf

        moveq   #1,d1           ; [d1] si
        tst.w   d2              ; dx >= 0 ?
        bge.s   .di_ok
        neg.w   d1
        neg.w   d4

.di_ok  swap    d4              ; [d4] d = (di << 16) | df
        swap    d1              ; [d1] (si << 16)
        clr.w   d1
        ext.l   d3
        sub.l   d3,d1           ; [d1] s = (si << 16) - dy
        move.w  d3,d2           ; [d2] n

.loop   move.l  d0,d5           ; (4)
        swap    d5              ; (4) [d5] xi

        move.w  d5,d6           ; (4)
        asr.w   #3,d6           ; (12)

        not.w   d5              ; (4)
        bchg.b  d5,(a0,d6.w)    ; (8+10)

        add.w   a1,a0           ; (8)
        add.l   d4,d0           ; (6) x += d
        cmp.w   d3,d0           ; (4) xf >= dy ?
        blt.s   .skip           ; (8/10)
        add.l   d1,d0           ; (6) x += s
.skip   dbf     d2,.loop        ; (10)

.exit:  movem.l (sp)+,d2-d6
        rts
