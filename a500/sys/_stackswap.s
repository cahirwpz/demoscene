        include "exec/execbase.i"
        include "exec/tasks.i"
        include "lvo/exec_lib.i"

        xdef __StackSwap

        section text, code

__StackSwap:
        movem.l a0/a6,-(sp)
        move.l  $4.w,a6
        ; Prevent OS to see inconsistent state
        jsr     _LVODisable(a6)
        suba.l  a1,a1
        jsr     _LVOFindTask(a6)
        move.l  d0,a1
        movem.l (sp)+,a0/a6

        ; Pop return address
        move.l  (sp)+,d0

        ; Now the stack is clean
        ; [d0] return address
        ; [a0] StackSwapStruct
        ; [a1] address of task

        lea     TC_SPLOWER(a1),a1

        move.l  (a1),d1
        move.l  (a0),(a1)+
        move.l  d1,(a0)+

        move.l  (a1),d1
        move.l  (a0),(a1)+
        move.l  d1,(a0)+

        move.l  sp,d1
        move.l  (a0),sp
        move.l  d1,(a0)

        ; Reenable interrupts
        movem.l d0/a6,-(sp)
        move.l  $4.w,a6
        jsr     _LVOEnable(a6)
        movem.l (sp)+,a0/a6

        ; [a0] return address
        jmp     (a0)

        ; vim: ft=asm68k:ts=8:sw=8
