        xdef    _StopCPU

        section "code",code

_StopCPU:
        stop    #$0200
        rte

; vim: ft=asm68k:ts=8:sw=8
