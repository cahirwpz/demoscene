; vim: ft=asm68k:ts=8:sw=8:

        section "distortion_opt", code

; a2   chunky buffer
; a3   struct DistortionMap
; a5   texture 256x256x8
; d0.b offsetX
; d1.b offsetY

        machine 68040

RenderDistortionRegs    reg     d2-d5/a2/a3/a5

        XDEF    _RenderDistortion

_RenderDistortion
        movem.l RenderDistortionRegs,-(sp)

        ; calculate offset
        and.l   #$ff,d1
        lsl.l   #8,d1
        or.l    d1,d0

        ; calculate number of iterations
        move.w  (a3),d1
        mulu.w  2(a3),d1
        lsr.l   #2,d1

        ; prepare temporary pointers
        move.l  4(a3),a0
        move.l  (a2),a2   ; CanvasT.pixbuf
        move.l  4(a2),a1  ; PixBufT.data
        move.l  4(a5),a5

        ; clear offset registers
        clr.l   d2
        clr.l   d3
        clr.l   d4
        clr.l   d5

.loop   move.w  (a0)+,d2
        move.w  (a0)+,d3
        move.w  (a0)+,d4
        move.w  (a0)+,d5
        add.w   d0,d2
        add.w   d0,d3
        add.w   d0,d4
        add.w   d0,d5
        move.b  (a5,d2.l),(a1)+
        move.b  (a5,d3.l),(a1)+
        move.b  (a5,d4.l),(a1)+
        move.b  (a5,d5.l),(a1)+
        subq.w  #1,d1
        bne.b   .loop

        movem.l (sp)+,RenderDistortionRegs
        rts
