        include "exec/execbase.i"
        include "exec/tasks.i"
        include "lvo/exec_lib.i"

        xdef    _NewStackSwap

        section text, code

; [a0] StackSwapStruct
_StackSwap:
        movem.l a0/a6,-(sp)
        move.l  $4.w,a6
        ; Prevent OS from seeing inconsistent state
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

_NewStackSwap:
        move.l  a2,-(sp)

        ; [0(a2)] struct StackSwapStruct
        ; [4(sp)] function pointer
        ; [8(a2)] struct StackSwapArgs
        lea     8(sp),a2

        move.l  (a2),a0
        bsr     _StackSwap

        move.l  8(a2),d1
        beq     .nopushargs

        ; Put the C arguments on the stack
        move.l  d1,a0
        lea     8*4(a0),a0
        moveq   #7,d0
.copy:
        move.l  -(a0),-(sp)
        dbf     d0,.copy

.nopushargs:
        ; Call the C function
        move.l  4(a2),a0
        jsr     (a0)

        move.l  8(a2),d1
        beq     .nopopargs

        ; Remove the C arguments
        lea.l   8*4(sp),sp

.nopopargs:
        ; save C function returncode
        move.l  (a2),a0
        move.l  d0,a2
        bsr     _StackSwap
        move.l  a2,d0

        movem.l (sp)+,a2
        rts

; vim: ft=asm68k:ts=8:sw=8
