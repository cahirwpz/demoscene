                include 'exec/macros.i'
                include 'dos/dosextens.i'
                include 'lvo/exec_lib.i'

                xdef    _exit
                xdef    __exit
                xdef    _geta4

                xdef    ___argc
                xdef    ___argv
                xdef    ___commandline
                xdef    ___commandlen
                xdef    ___SaveSP
                xdef    _SysBase

                xref    _main
                xref    ___INIT_LIST__
                xref    ___EXIT_LIST__

                section '.text', code

start:
                move.l  a0,___commandline
                move.l  d0,___commandlen

                move.l  sp,___SaveSP
                move.l  $4.w,a6
                move.l  a6,_SysBase

                suba.l  a1,a1
                JSRLIB  FindTask
                move.l  d0,a3
                tst.l   pr_CLI(a3)
                bne     fromCLI

fromWB:         lea     pr_MsgPort(a3),a0
                JSRLIB  WaitPort
                lea     pr_MsgPort(a3),a0
                JSRLIB  GetMsg
                move.l  d0,__WBenchMsg

fromCLI:        lea     ___INIT_LIST__+4,a2
                moveq.l #-1,d2
                bsr     callfuncs
                move.l  ___env,-(sp)
                move.l  ___argv,-(sp)
                move.l  ___argc,-(sp)
                jsr     _main
                move.l  d0,4(sp)

_exit:
__exit:         lea     ___EXIT_LIST__+4,a2
                moveq.l #0,d2
                bsr     callfuncs

                move.l  $4.w,a6

                move.l  __WBenchMsg,d2
                beq     todos
                JSRLIB  Forbid
                move.l  d2,a1
                JSRLIB  ReplyMsg

todos:          move.l  4(sp),d0
                move.l  ___SaveSP,sp
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

___commandline: ds.l    1
___commandlen:  ds.l    1
___SaveSP:      ds.l    1
_SysBase:       ds.l    1
__WBenchMsg:    ds.l    1
___argc:        ds.l    1
___argv:        ds.l    1
___env:         ds.l    1

cleanupflag:    ds.l    1

; vim: ft=asm68k:ts=8:sw=8
