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
        beq  	.exit           ; return if dy == 0

        move.w  a1,d4
        muls.w  d1,d4
        move.w  d0,d5
        asr.w   #3,d5
        add.w   d4,d5
        add.w   d5,a0           ; pixels += ys * stride + xs / 8

        move.w  d2,d4           ; [d4] adx = abs(dx)
        bge.s   .dx_ok
        neg.w   d4

.dx_ok                          ; [d4] df = adx
        clr.w   d5              ; [d5] di = 0
        cmp.w   d3,d4
        blt.s   .no_div
        divs.w  d3,d4
        move.w  d4,d5           ; [d5] di = adx / dy
        swap    d4              ; [d4] df = adx % dy

.no_div move.w  a1,d1           ; [d1] dp = stride
        not.w   d0
        and.w   #7,d0           ; [d0] xi = ~xs & 7
        move.w  d3,d6
        neg.w   d6              ; [d6] xf = -dy

        tst.w   d2              ; dx >= 0 ?
        blt.s   .case2

.case1  move.w  d5,d2
        asr.w   #3,d2
        add.w   d2,d1           ; dp += di / 8
        and.w   #7,d5           ; di = di & 7
        move.w  d3,d2           ; [d2] n = dy
        subq.w  #1,d2

        ; min/max cycles per iteration: 56/76
.loop1  bchg    d0,(a0)         ; (12) put pixel
        add.w   d1,a0           ; (8) ptr += dp
        add.w   d4,d6           ; (4) xf += df
        blt.s   .xf1            ; (8/10) xf >= 0 ?
        subq.w  #1,d0           ; (4) xi--
        sub.w   d3,d6           ; (4) xf -= dy
.xf1    sub.w   d5,d0           ; (4) xi -= di
        blt.s   .xi1            ; (8/10) xi < 0 ?
        dbf     d2,.loop1       ; (10)
        bra.s   .exit

.xi1    addq.l  #1,a0           ; (8) ptr++
        addq.w  #8,d0           ; (4) xi += 8
        dbf     d2,.loop1       ; (10)
        bra.s   .exit

.case2  move.w  d5,d2
        asr.w   #3,d2
        sub.w   d2,d1           ; dp -= di / 8
        and.w   #7,d5
        neg.w   d5              ; di = -(di & 7)
        subq.w  #8,d0           ; xi -= 8
        move.w  d3,d2           ; [d2] n = dy
        subq.w  #1,d2

        ; min/max cycles per iteration: 56/76
.loop2  bchg    d0,(a0)         ; (12) put pixel
        add.w   d1,a0           ; (8) ptr += dp
        add.w   d4,d6           ; (4) xf += df
        blt.s   .xf2            ; (8/10) xf >= 0 ?
        addq.w  #1,d0           ; (4) xi++
        sub.w   d3,d6           ; (4) xf -= dy
.xf2    sub.w   d5,d0           ; (4) xi -= di
        bge.s   .xi2            ; (8/10) xi >= 0 ?
        dbf     d2,.loop2       ; (10)
        bra.s   .exit

.xi2    subq.l  #1,a0           ; (8) ptr--
        subq.w  #8,d0           ; (4) xi -= 8
        dbf     d2,.loop2       ; (10)

.exit:  movem.l (sp)+,d2-d6
        rts
