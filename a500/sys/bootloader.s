        include 'exec/macros.i'
        include 'exec/resident.i'
        include 'devices/bootblock.i'
        include 'lvo/exec_lib.i'
        include 'hardware/custom.i'

	section	BB,code

; Boot block cannot generally be assumed to be placed in a specific kind of
; memory, or at any specific address, so all code must be completely PC
; relative, and all chip data must be explicitly copied to chip memory.

BB_START
	dc.l	BBNAME_DOS
	dc.l	0
	dc.l	0

        ; Boot block entry:
        ;  [a1] IOStdReq
        ;  [a6] ExecBase
BB_CODE

BB_EXIT
        ; Boot block exit:
        ;  [d0] boot error code
        ;  [a0] initialization function

        lea     DOSNAME(pc),a1
        JSRLIB  FindResident
        tst.l   d0
        beq     .error
        move.l  d0,a0
        move.l  RT_INIT(a0),a0
        moveq   #0,d0
        rts

.error  moveq   #-1,d0
        rts

DOSNAME dc.b    'dos.library'

; vim: ft=asm68k:ts=8:sw=8
