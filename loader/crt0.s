        xdef    _start

        xref    ___INIT_LIST__
        xref    ___EXIT_LIST__
        xref    _CallFuncList
        xref    _main

        section '.text', code

_start: lea     ___INIT_LIST__,a0
        jsr     _CallFuncList

        jsr     _main

_exit:  lea     ___EXIT_LIST__,a0
        jsr     _CallFuncList

        bra     *

; vim: ft=asm68k:ts=8:sw=8
