        xdef    _start
        xref    _Loader

        section '.text', code

_start: jsr     _Loader
        bra     *

; vim: ft=asm68k:ts=8:sw=8
