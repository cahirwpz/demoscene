        xdef    _GetVBR
        xdef    _StopCPU
        xdef    _KPutChar
        xdef    _InterruptVector

        include 'hardware/custom.i'

        section "code",code

_GetVBR:
        movec   vbr,d0
        rte

_StopCPU:
        stop    #$2000
        rte

_KPutChar:
        and.w   #$ff,d0
        or.w    #$100,d0

.loop   btst    #5,serdatr+$dff000
        beq     .loop

        move.w  d0,serdat+$dff000

        cmp.b   #10,d0  ; '\n'
        bne     .quit

        addq.w  #3,d0   ; '\r'
        bra     .loop

.quit:  rts

        section "bss",bss

_InterruptVector:
        ds.l    1

; vim: ft=asm68k:ts=8:sw=8
