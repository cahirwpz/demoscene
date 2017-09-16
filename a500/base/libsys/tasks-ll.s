; Low level extensions to exec.library API.

        include 'exec/macros.i'
        include 'exec/types.i'
        include 'exec/lists.i'
        include 'exec/execbase.i'
        include 'exec/tasks.i'
        include 'lvo/exec_lib.i'

        xdef    _TaskYield
        xdef    _TaskWait
        xdef    _TaskSignal
        xdef    _TaskSignalIntr

; Please read exec.library disassembly by Markus Wandel
; to learn about undocumented functions like Switch() and Schedule().

_LVOExitIntr    EQU     -36
_LVOSchedule    EQU     -42
_LVOReschedule  EQU     -48
_LVOSwitch      EQU     -54
_LVODispatch    EQU     -60
_LVOException   EQU     -66

; SysBase->SysFlags undocumented flags related to scheduling.

	BITDEF	SF,TQE,14	; Time Quantum Elapsed
	BITDEF	SF,SAR,15	; Scheduler Attention Request

        section code,code

; TaskYield must be executed in user mode!
_TaskYield:
        movem.l a5/a6,-(sp)
        lea     _TaskYieldSV(pc),a5
        move.l  $4.w,a6
        JSRLIB  Supervisor
        movem.l (sp)+,a5/a6
        rts

_TaskYieldSV:
        ; Fetch address of currently running task.
        move.l  ThisTask(a6),a1
        ; Mark the task as ready to be run.
        move.b  #TS_READY,TC_STATE(a1)
        ; Put it on a ready task queue.
        lea     TaskReady(a6),a0
        JSRLIB  Enqueue
        ; Force a task switch by calling Switch.
        jmp     _LVOSwitch(a6)

; TaskWait must be executed in user mode!
; a0 [struct List *] event
_TaskWait:
        movem.l a5/a6,-(sp)
        lea     _TaskWaitSV(pc),a5
        move.l  $4.w,a6
        JSRLIB  Supervisor
        movem.l (sp)+,a5/a6
        rts

; Waiting tasks are not put into SysBase->TaskWait list!
_TaskWaitSV:
        ; Fetch address of currently running task.
        move.l  ThisTask(a6),a1
        ; Mark the task as suspended.
        move.b  #TS_WAIT,TC_STATE(a1)
        ; Put it on a event wait list.
        ; A0 : list (destroyed), A1 : node, D0 (destroyed)
        ADDTAIL
        ; Force a task switch.
        jmp     _LVOSwitch(a6)

; TaskSignal must be executed only in user mode!
; a0 [struct List *] event
_TaskSignal:
        ; Quit if the list of waiting tasks is empty.
        IFEMPTY a0,.quit

        movem.l a2/a5/a6,-(sp)
        ; Fetch ExecBase
        move.l  $4.w,a6
        ; Disable interrupts!
        JSRLIB  Disable
        ; Fetch first task.
        move.l  LH_HEAD(a0),a2
        ; Clear the list by reinitializing it.
        NEWLIST a0

.loop   ; Mark it as ready to run.
        move.b  #TS_READY,TC_STATE(a2)
        ; Put it on a ready task queue.
        lea     TaskReady(a6),a0
        move.l  a2,a1
        JSRLIB  Enqueue
        ; Repeat if there's next task to wake up.
        TSTNODE a2,a2
        bne     .loop

        ; Enable interrupts!
        JSRLIB  Enable
        ; Force the scheduler to take action!
        lea     _LVOSchedule(a6),a5
        JSRLIB  Supervisor
        movem.l (sp)+,a2/a5/a6

.quit   rts


; TaskSignalIntr must be executed only in interrupt context!
; a0 [struct List *] event
_TaskSignalIntr:
        ; Quit if the list of waiting tasks is empty.
        IFEMPTY a0,.quit

        movem.l a2/a6,-(sp)
        ; Fetch ExecBase
        move.l  $4.w,a6
        ; Fetch first task.
        move.l  LH_HEAD(a0),a2
        ; Clear the list by reinitializing it.
        NEWLIST a0

.loop   ; Mark it as ready to run.
        move.b  #TS_READY,TC_STATE(a2)
        ; Put it on a ready task queue.
        lea     TaskReady(a6),a0
        move.l  a2,a1
        JSRLIB  Enqueue
        ; Repeat if there's next task to wake up.
        TSTNODE a2,a2
        bne     .loop
        
        ; Force a scheduler to take action while returning from interrupt!
        bset.b  #SFB_SAR-8,SysFlags(a6)
        movem.l (sp)+,a2/a6

.quit   rts

; vim: ft=asm68k:ts=8:sw=8
