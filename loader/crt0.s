                xdef    _start
                xdef    _exit
                xdef    _geta4
                xdef    _SysBase

                xref    ___INIT_LIST__
                xref    ___EXIT_LIST__
                xref    _CallFuncList
                xref    _main

                section '.text', code

_start:         lea     ___INIT_LIST__,a0
                jsr     _CallFuncList

                jsr     _main

_exit:          lea     ___EXIT_LIST__,a0
                jsr     _CallFuncList

                moveq.l #0,d0
                rts

; geta4() doesn´t do anything, but enables you to use
; one source for both code models

_geta4:         rts

; vim: ft=asm68k:ts=8:sw=8
