                include 'exec/macros.i'
                include 'dos/dosextens.i'
                include 'lvo/exec_lib.i'

                xdef    _exit
                xdef    _geta4

                xdef    _SysBase

                xref    _main
                xref    ___INIT_LIST__
                xref    ___EXIT_LIST__

                section '.text', code

start:          move.l  $4.w,a6
                move.l  a6,_SysBase

                lea     ___INIT_LIST__+4,a2
                moveq.l #-1,d2
                bsr     callfuncs

                jsr     _main

_exit:          lea     ___EXIT_LIST__+4,a2
                moveq.l #0,d2
                bsr     callfuncs

                moveq.l #0,d0
                rts

; call all functions in the NULL terminated list pointed to by a2
; d2 ascending or descending priority mode

callfuncs:      lea     cleanupflag,a5
                move.l  a2,a3
                moveq.l #0,d3
                bra     oldpri
stabloop:       move.l  (a3)+,d4
                move.l  (a5),d5
                cmp.l   d4,d5
                bne     notnow
                move.l  d0,a0
                jsr     (a0)
notnow:         eor.l   d2,d4
                eor.l   d2,d5
                cmp.l   d5,d4
                bcc     oldpri
                cmp.l   d3,d4
                bls     oldpri
                move.l  d4,d3
oldpri:         move.l  (a3)+,d0
                bne     stabloop
                eor.l   d2,d3
                move.l  d3,(a5)
                cmp.l   d2,d3
                bne     callfuncs

; geta4() doesn´t do anything, but enables you to use
; one source for both code models

_geta4:         rts

                section '.bss', bss

_SysBase:       ds.l    1
cleanupflag:    ds.l    1

; vim: ft=asm68k:ts=8:sw=8
