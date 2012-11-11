        xdef    _GetVBR
        xdef    _InterruptVector

        section "code",code

_GetVBR:
        movec   vbr,d0
        rte

        section "bss",bss

_InterruptVector:
        ds.l    1

; vim: ft=asm68k:ts=8:sw=8
