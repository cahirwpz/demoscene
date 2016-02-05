        include 'exec/macros.i'
        include 'exec/io.i'
        include 'exec/memory.i'
        include 'dos/dosextens.i'
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
DirLen: dc.l	0               ; sector aligned

        ; Boot block entry:
        ;  [a1] IOStdReq (trackdisk.device)
        ;  [a6] ExecBase
Entry:
        lea     TDIOReq(pc),a0
        move.l  a1,(a0)

        ; some resident modules have not been initialized yet,
        ; one that is used by my code is mathffp.library
        lea     ffpName(pc),a1
        JSRLIB  FindResident

        move.l  d0,a1
        clr.l   d1
        JSRLIB  InitResident

        ; allocate memory for directory entries
        move.l  DirLen(pc),d0
        move.l  #MEMF_CHIP,d1
        JSRLIB  AllocMem

        ; [a2] directory entires
        move.l  d0,a2

        ; read directory entries into memory
        move.l  DirLen(pc),d0
        moveq.l #4,d1
        lsl.l   #8,d1
        move.l  a2,a0
        bsr     ReadSectors

        ; scan for executable files
        move.l  (a2)+,d7
        swap    d7

.loop   ; [d2] file offset 
        ; [d3] file size (executable)
        movem.l (a2)+,d2-d3     ; offset, size
        add.l   #TD_SECTOR-1,d3
        and.l   #-TD_SECTOR,d3
        tst.l   d2              ; executable ?
        bge     .next

.exec   neg.l   d2

        ; allocate memory for executable file
        move.l  d3,d0
        move.l  #MEMF_CHIP,d1
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

        ; make executable startup think it was called from CLI
        sub.l   a1,a1
        JSRLIB  FindTask
        move.l  d0,a0
        move.l  #$DEADC0DE,pr_CLI(a0)

        ; call executable
        lea     .cmd(pc),a0
        moveq   #1,d0
        movem.l d2-d7/a2-a6,-(sp)
        jsr     SEG_START(a4)
        movem.l (sp)+,d2-d7/a2-a6

        ; free hunk list
.free   move.l  a4,a1
        movem.l (a1),d0/a4              ; SEG_LEN / SEG_NEXT
        JSRLIB  FreeMem
        move.l  a4,d0
        bne     .free

.next   subq.w  #1,d7
        bgt     .loop

.exit   bra     .exit

.cmd    dc.b    '\n',0

ffpName dc.b    'mathffp.library',0

; [d0] length in bytes (multiple of 512)
; [d1] offset from beginning of disk
; [a0] destination buffer (must be in chip memory)
ReadSectors:
        move.l  TDIOReq(pc),a1
        move.w  #CMD_READ,IO_COMMAND(a1)
        exg.l   d1,a0
        movem.l d0/d1/a0,IO_LENGTH(a1)  ; length / data / offset
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
        lea     4(a0),a2

        ; read number of hunks, assume there's no resident library name
        move.l  (a2)+,d2
        lsl.l   #2,d2           ; [d2] hunk array size
        sub.l   d2,sp           ; [sp] hunk array

        ; move to hunk information
        addq.l  #8,a2

        ; allocate hunks
        move.l  sp,a3
        move.l  d2,d4
.alloc  move.l  (a2)+,d3
        lsl.l   #2,d3           ; [d3] hunk size
        addq.l  #SEG_SIZE,d3
        move.l  d3,d0
        move.l  #MEMF_PUBLIC|MEMF_CLEAR,d1
        JSRLIB  AllocMem
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
        move.l  (a1),SEG_NEXT(a0)
        subq.l  #4,d4
        bgt     .link

        ; parse hunks
        move.l  sp,a3
        move.l  d2,d4
.parse  move.w  (a2)+,d0        ; should always read zero
        add.w   (a2)+,d0        ; hunk type
        cmp.w   #HUNK_CODE,d0
        beq     .hdata
        cmp.w   #HUNK_DATA,d0
        beq     .hdata
        cmp.w   #HUNK_BSS,d0
        beq     .hbss
        cmp.w   #HUNK_RELOC32,d0
        beq     .hreloc
        cmp.w   #HUNK_SYMBOL,d0
        beq     .hsyms
        cmp.w   #HUNK_END,d0
        beq     .hend
        moveq   #0,d0           ; report error
        bra     .error

.hdata  move.l  (a3),a1
        lea.l   SEG_START(a1),a1
        move.l  (a2)+,d0
        lsl.l   #2,d0
        move.l  a2,a0
        add.l   d0,a2           ; move pointer to next hunk
        JSRLIB  CopyMem
        bra     .parse

.hbss   addq.l  #4,a2           ; skip bss length
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
        addq.l  #SEG_START,d0    ; [d0] referenced hunk data address
        move.l  (a3),a1
.reloc  move.l  (a2)+,d1
        add.l   d0,SEG_START(a1,d1.l)
        subq.l  #1,d3
        bgt     .reloc
        bra     .hreloc

.hend   addq.l  #4,a3           ; go to next hunk
        subq.l  #4,d4
        bne     .parse

.quit   move.l  (sp),d0

.error  add.l   d2,sp           ; deallocate hunk array

        movem.l (sp)+,d2-d4/a2-a3
        rts

; vim: ft=asm68k:ts=8:sw=8
