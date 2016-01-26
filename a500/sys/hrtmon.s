        xdef    _CallHRTmon

        section code,code

_CallHRTmon:
        lea     4(sp),sp
        jmp     $a1000c

; vim: ft=asm68k:ts=8:sw=8
