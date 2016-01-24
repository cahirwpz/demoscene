        include 'exec/macros.i'
        include 'exec/io.i'
        include 'exec/memory.i'
        include 'dos/doshunks.i'
        include 'devices/bootblock.i'
        include 'devices/trackdisk.i'
        include 'lvo/exec_lib.i'
        include 'hardware/custom.i'

 STRUCTURE SEG,0
	LONG	SEG_LEN
	APTR	SEG_NEXT
	LABEL	SEG_START
	LABEL	SEG_SIZE

	section	BB,code

; Boot block cannot generally be assumed to be placed in a specific kind of
; memory, or at any specific address, so all code must be completely PC
; relative, and all chip data must be explicitly copied to chip memory.

	dc.l	BBNAME_DOS
	dc.l	0
DirLen: dc.l	0

        ; Boot block entry:
        ;  [a1] IOStdReq (trackdisk.device)
        ;  [a6] ExecBase
Entry:
        ; [a5] custom base
        lea     $dff000,a5
        clr.w   color(a5)

        lea     TDIOReq(pc),a0
        move.l  a1,(a0)

        ; allocate memory for directory entries
        move.l  DirLen(pc),d0
        bsr     AlignToSector
        move.l  #MEMF_CHIP|MEMF_CLEAR,d1
        JSRLIB  AllocMem

        ; [a2] directory entires
        move.l  d0,a2

        ; read directory entries into memory
        move.l  DirLen(pc),d0
        bsr     AlignToSector
        move.l  #1024,d1
        move.l  a2,a0
        bsr     ReadSectors

        ; scan for executable files
        move.l  (a2)+,d7
        swap    d7
        subq.w  #1,d7

.loop   movem.l (a2)+,d2-d3     ; offset, size
        tst.l   d2              ; executable ?
        bge     .next

.exec   neg.l   d2

        ; allocate memory for executable file
        move.l  d3,d0
        bsr     AlignToSector
        move.l  d0,d3           ; [d3] block size (executable file)
        move.l  #MEMF_CHIP|MEMF_CLEAR,d1
        JSRLIB  AllocMem
        move.l  d0,a3           ; [a3] block pointer (executable file)

        ; read directory entries into memory
        move.l  d2,d1
        move.l  d3,d0
        move.l  a3,a0
        bsr     ReadSectors

        ; move hunks around and relocate them
        move.l  a3,a0
        bsr     SetupHunkFile
        move.l  d0,a4           ; [a4] first hunk of executable file

        ; free executable file
        move.l  a3,a1
        move.l  d3,d0
        JSRLIB  FreeMem

        move.l  a4,d0           ; oops... it failed
        beq.b   .next

        ; call executable
        lea     .cmd(pc),a0
        moveq   #1,d0
        jsr     SEG_START(a4)

        ; free hunk list
        move.l  a4,a0
        bsr     FreeHunkFile

.next   dbf     d7,.loop

.exit   bra     .exit

.cmd    dc.b    '\n',0

; [d0] size to aligned
AlignToSector:
        add.l   #TD_SECTOR-1,d0
        and.l   #-TD_SECTOR,d0
        rts

; [d0] length in bytes (multiple of 512)
; [d1] offset from beginning of disk
; [a0] destination buffer (must be in chip memory)
ReadSectors:
        move.l  TDIOReq(pc),a1
        move.w  #CMD_READ,IO_COMMAND(a1)
        move.l  d0,IO_LENGTH(a1)
        move.l  d1,IO_OFFSET(a1)
        move.l  a0,IO_DATA(a1)
        JSRLIB  DoIO
        rts

TDIOReq dc.l    0

SetupHunkFile:
        ; executable amiga hunk file?
        cmp.l   #HUNK_HEADER,(a0)+
        beq     .setup
        moveq   #0,d0
        rts

.setup  movem.l d2-d4/a2-a3,-(sp)
        move.l  a0,a2

        ; read number of hunks, assume there's no resident library name
        move.l  4(a2),d2
        lsl.l   #2,d2           ; [d2] hunk array size
        sub.l   d2,sp           ; [sp] hunk array

        ; move to hunk information
        lea     16(a2),a2

        ; allocate hunks
        move.l  sp,a3           ; [a3] hunk array
        move.l  d2,d4
.alloc  move.l  (a2)+,d3
        lsl.l   #2,d3           ; [d3] hunk size
        move.l  d3,d0
        addq    #SEG_SIZE,d0
        move.l  #MEMF_PUBLIC|MEMF_CLEAR,d1
        JSRLIB  AllocMem
        bsr     PutLong
        move.l  d0,(a3)+
        move.l  d0,a0
        move.l  d3,SEG_LEN(a0)
        subq.l  #4,d4
        bgt     .alloc

        ; link hunks
        move.l  sp,a1
        move.l  d2,d4
        subq.l  #4,d4
.link   move.l  (a1)+,a0        ; previous hunk
        move.l  (a1),d0
        addq.l  #SEG_NEXT,d0
        move.l  d0,SEG_NEXT(a0)
        subq.l  #4,d4
        bgt     .link

        ; parse hunks
        move.l  sp,a3
        move.l  d2,d4
.parse  move.l  (a2)+,d0        ; hunk type
        cmp.l   #HUNK_CODE,d0
        beq     .hdata
        cmp.l   #HUNK_DATA,d0
        beq     .hdata
        cmp.l   #HUNK_BSS,d0
        beq     .hbss
        cmp.l   #HUNK_RELOC32,d0
        beq     .hreloc
        cmp.l   #HUNK_SYMBOL,d0
        beq     .hsyms
        cmp.l   #HUNK_END,d0
        beq     .hend
        moveq   #0,d0           ; report error
        bra     .quit

.hdata  move.l  (a3),a1
        lea.l   SEG_START(a1),a1
        move.l  (a2)+,d0
        lsl.l   #2,d0
        move.l  a2,a0
        add.l   d0,a2           ; move pointer to next hunk
        JSRLIB  CopyMem
        bra     .parse

.hbss   lea     4(a2),a2        ; skip bss length
        bra     .parse

.hsyms  move.l  (a2)+,d0
        beq     .parse
        addq.l  #1,d0
        lsl.l   #2,d0
        add.l   d0,a2           ; move pointer to symbol
        bra     .hsyms

.hreloc move.l  (a2)+,d3        ; number of relocations
        beq     .parse
        move.l  (a2)+,d0        ; referenced hunk number
        lsl.l   #2,d0
        move.l  (sp,d0.l),d0
        add.l   #SEG_START,d0    ; [d0] referenced hunk data address
        move.l  (a3),a1
.reloc  move.l  (a2)+,d1
        add.l   d0,SEG_START(a1,d1.l)
        subq.l  #1,d3
        bgt     .reloc
        bra     .hreloc

.hend   addq    #4,a3           ; go to next hunk
        subq    #4,d4
        bne     .parse

.quit   move.l  (sp),d0

        add.l   d2,sp           ; deallocate hunk array

        movem.l (sp)+,d2-d4/a2-a3
        rts

FreeHunkFile:
        rts

PutChar:
        and.w   #$ff,d0
        or.w    #$100,d0
.loop   btst    #5,serdatr(a5)
        beq     .loop
        move.w  d0,serdat(a5)
        cmp.b   #10,d0  ; '\n'
        bne     .quit
        addq.w  #3,d0   ; '\r'
        bra     .loop
.quit:  rts

PutLong:
        movem.l d0-d1/a0,-(sp)
        moveq   #7,d1
        lea     .hex(pc),a0
.loop   rol.l   #4,d0
        move.l  d0,-(sp)
        and.w   #$000f,d0
        move.b  (a0,d0.w),d0
        bsr     PutChar
        move.l  (sp)+,d0
        dbf     d1,.loop
        moveq   #'\n',d0
        bsr     PutChar
        movem.l (sp)+,d0-d1/a0
        rts

.hex    dc.b    "0123456789ABCDEF"

DOSNAME dc.b    'dos.library'

; vim: ft=asm68k:ts=8:sw=8
