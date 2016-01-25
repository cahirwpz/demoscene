        xdef    _GetVBR
        xdef    _StopCPU
        xdef    _InterruptVector

        include 'hardware/custom.i'

        section "code",code

_GetVBR:
        movec   vbr,d0
        rte

_StopCPU:
        stop    #$2000
        rte

        section "bss",bss

_InterruptVector:
        ds.l    1

; vim: ft=asm68k:ts=8:sw=8
