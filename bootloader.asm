; Copyright (C) 2020-2024 Krystian Bacławski
;
; Permission is hereby granted, free of charge, to any person obtaining a copy
; of this software and associated documentation files (the "Software"), to deal
; in the Software without restriction, including without limitation the rights
; to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
; copies of the Software, and to permit persons to whom the Software is
; furnished to do so, subject to the following conditions:
;
; The above copyright notice and this permission notice shall be included in all
; copies or substantial portions of the Software.
;
; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
; IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
; AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
; LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
; OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
; SOFTWARE.

        include 'exec/macros.i'
        include 'exec/io.i'
        include 'exec/execbase.i'
        include 'exec/memory.i'
        include 'exec/libraries.i'
        include 'dos/dosextens.i'
        include 'dos/doshunks.i'
        include 'devices/trackdisk.i'
        include 'hardware/custom.i'

; Export some symbols so they appear in disassembly.

        XDEF    Start
        XDEF    Panic
        XDEF    CopyMem
        XDEF    ClearMem
        XDEF    AllocMem
        XDEF    FreeMem
        XDEF    SetupHunkFile

; configurable parameters

STACKSIZE       EQU     512     ; initial stack size for the program
CHIPMAX         EQU     $200000 ; maximum size of chip memory (2MB)

; custom chips addresses

CUSTOM          EQU     $dff000
CIAA            EQU     $bfe001
CIAB            EQU     $bfd000

; jump vector entires from exec.library

_LVOSuperState          EQU     -150
_LVOAllocMem            EQU     -198
_LVODoIO                EQU     -456
_LVOCacheControl        EQU     -648

; These flags are not related to AmigaOS memory management.

        BITDEF  M,CHIP,0        ; block must be in chip memory
        BITDEF  M,CLEAR,1       ; clear allocated block
        BITDEF  M,REVERSE,2     ; allocate from the top down

; Boot loader data definition

 STRUCTURE BD,0                 ; Boot Data
        APTR    BD_HUNK
        APTR    BD_VBR
        APTR    BD_STKBOT
        LONG    BD_STKSZ
        BYTE    BD_BOOTDEV
        BYTE    BD_CPUMODEL
        WORD    BD_NREGIONS
        LABEL   BD_REGION
        LABEL   BD_SIZE

 STRUCTURE MR,0                 ; Memory Region
        APTR    MR_LOWER
        APTR    MR_UPPER
        WORD	MR_ATTR
        LABEL   MR_SIZE

 STRUCTURE SEG,0                ; Amiga Hunk
        LONG	SEG_LEN 
        APTR	SEG_NEXT
        LABEL	SEG_START
        LABEL	SEG_SIZE

        ifnd    ROM

        XDEF    Entry
        XDEF    KillOS

; Boot block cannot generally be assumed to be placed in a specific kind of
; memory, or at any specific address, so all code must be completely PC
; relative, and all chip data must be explicitly copied to chip memory.

        section	'.text',code

        dc.b    'DOS',0
        dc.l	0

        ; Executable file image information
        ; length / start (sector aligned, shifted right by 8)
ExecInfo:
        dc.w    0, 0

; AmigaOS loads boot block somewhere at the beginning of chip memory and 
; jumps in here.
;
; Let's use OS to load executable file into memory, and kill it afterwards.
;
; Useful registers provided by AmigaOS:
;   [a1] IOStdReq (trackdisk.device)
;   [a6] ExecBase

Entry:
        move.l  a1,-(sp)                ; trackdisk.device IORequest

        movem.w ExecInfo(pc),d2/d4
        lsl.l   #8,d2                   ; [d2] executable length in bytes
        lsl.l   #8,d4                   ; [d4] executable start in bytes

        ; allocate memory for executable file
        move.l  d2,d0
        moveq.l #MEMF_CHIP,d1
        JSRLIB  AllocMem
        move.l  d0,d3                   ; [d3] block pointer (executable file)

        ; read executable file into memory
        move.l  (sp)+,a1                ; trackdisk.device IORequest
        movem.l d2-d4,IO_LENGTH(a1)     ; length / data / start 
        JSRLIB  DoIO

        ; turn off the motor
        clr.l   IO_LENGTH(a1)
        move.w  #TD_MOTOR,IO_COMMAND(a1)
        JSRLIB  DoIO

        ; disable caches if running on kickstart 2.0 or above
        cmp.w   #36,LIB_VERSION(a6)
        blt     .nocache
        moveq.l #0,d0
        moveq.l #-1,d1
        JSRLIB  CacheControl
.nocache

; Let's kill AmigaOS before we normalize memory layout. That involves:
;
; 1) Destroying initial AmigaOS interrupt vector, that's always there,
;    even if it vector base register was changed on A3000 or A4000.
; 2) Moving out crucial data from ExecBase before it's smashed.
;
; Useful registers we carry to this phase:
;   [d2] executable length in bytes (rounded up to sector size)
;   [d3] pointer to executable file image
;
; We cannot yet normalize executable file image position since that could
; overwrite currently running code.

KillOS:
        move.w  #$7fff,intena+CUSTOM    ; disable all interrupts
        move.w  #$7fff,dmacon+CUSTOM    ; disable all DMA channels
        JSRLIB  SuperState              ; enter supervisor mode
        or.w    #$0700,sr               ; set highest priority level

        ; copy boot loader at $8.w
        lea     Start(pc),a0
        lea     $8.w,a2
        move.w  #(End-Start)/2-1,d0
.loop   move.w  (a0)+,(a2)+
        dbra    d0,.loop

        ; Useful data extracted from ExecBase includes memory regions
        ; and processor model flags.
        ;
        ; BOOTDATA structure above is placed just after the end of boot loader.

        ; [a2] boot loader data
        lea     BD_BOOTDEV(a2),a3

        ; start from floppy drive
        clr.b   (a3)+
        ; save processor model
        move.b  AttnFlags+1(a6),(a3)+

        ; save memory regions
        clr.w   (a3)+                   ; leave space for #regions
        lea     MemList(a6),a0          ; list of MemHeader structures
        move.l  a0,a1
.memory move.l  LN_SUCC(a1),a1
        ; check if memory region is MEMF_PUBLIC
        btst.b  #0,MH_ATTRIBUTES+1(a1)
        beq.b   .skipmh
        ; extract lower and upper memory region address
        move.w  MH_LOWER(a1),(a3)+
        clr.w   (a3)+                   ; lower address rounded down to 2^16
        move.l  MH_UPPER(a1),d0
        add.l   #65535,d0
        clr.w   d0
        move.l  d0,(a3)+                ; upper address rounded up to 2^16
        add.w   #1,BD_NREGIONS(a2)      ; increase number of regions
        move.w  MH_ATTRIBUTES(a1),(a3)+
.skipmh cmp.l   LH_TAILPRED(a0),a1
        bne     .memory

        ; copy boot loader data into safe register
        move.l  a2,a6
        suba.l  a2,a3                   ; [a3] size of boot loader data

        ; set up temporary stack at $400
        lea     $400.w,sp

        ; jump into second phase code relocated at the beginning of memory
        jmp     $8.w

        endif

; Normalize memory layout by moving executable file image at well known
; position. Set up primitive memory manager and relocate executable file.
;
; Useful registers we carry to this phase:
;   [d2] executable length in bytes (rounded up to sector size)
;   [d3] pointer to executable file image
;   [a3] size of boot loader data
;   [a6] boot loader data (cpu model, memory regions)
;   [sp] temporary stack at $400

Start:
        ; reserve space for the boot loader
        move.l  #$400,d0
        moveq.l #MF_CHIP,d1
        bsr     AllocMem

        ; by default exception vector address is at $0
        clr.l   d0

        ; relocate exception vector if cpu has VBR register (68010 or later)
        tst.b   BD_CPUMODEL(a6)
        beq.b   .novbr

        ; if two regions or more we have fast memory
        cmp.w   #2,BD_NREGIONS(a6)
        blt.b   .setvbr

        ; reserve space for exception vector in fast memory
        move.l  #$400,d0
        moveq   #MF_REVERSE|MF_CLEAR,d1
        bsr     AllocMem

.setvbr movec   d0,vbr

.novbr  move.l  d0,BD_VBR(a6)

        ifnd    ROM
        ; reserve space for executable file image
        move.l  d2,d0
        moveq   #MF_CHIP,d1
        bsr     AllocMem

        ; copy executable file image into reserved space
        move.l  d0,-(sp)
        move.l  d0,a1
        move.l  d3,a0
        move.l  d2,d0
        bsr     CopyMem
        move.l  (sp)+,d3
        endif

        ; move hunks around and relocate them
        move.l  d3,a0
        bsr     SetupHunkFile
        move.l  d0,BD_HUNK(a6)          ; first hunk of executable file

        ; free up space taken by executable file image
        ifnd    ROM
        move.l  d3,a0
        move.l  d2,d0
        bsr     FreeMem
        endif

        ; if VBR was relocated free space used by initial exception vector
        tst.l   BD_VBR(a6)
        beq.b   .vbr0
        suba.l  a0,a0
        move.l  #$400,d0
        bsr     FreeMem
.vbr0
        ; allocate initial stack for loaded program
        move.l  #STACKSIZE,BD_STKSZ(a6)
        move.l  BD_STKSZ(a6),d0
        moveq   #MF_CLEAR|MF_REVERSE,d1
        bsr     AllocMem
        move.l  d0,sp
        move.l  sp,BD_STKBOT(a6)
        add.l   BD_STKSZ(a6),sp

        ; copy boot data onto stack
        suba.l  a3,sp
        move.l  a6,a0
        move.l  sp,a1
        move.l  a3,d0
        bsr     CopyMem

        ; enter the kernel with pointer to boot data as first argument
        move.l  sp,a0
        move.l  BD_HUNK(a6),a1          ; [a1] first hunk of executable file
        jsr     SEG_START(a1)

; Something failed or kernel returned back to boot loader.
; Set background color to red and halt the processor.
Panic:
        move.w  #$f00,CUSTOM+color
        stop    #$2700

; Copies memory.
;
; Arguments:
;   [a0] pointer to source block
;   [a1] pointer to destination block
;   [d0] number of 4B words

CopyMem:
.loop   move.l  (a0)+,(a1)+
        subq.l  #4,d0
        bgt     .loop
        rts

; Clears memory.
; 
; Arguments:
;   [a0] pointer to cleared block
;   [d0] number of 4B words

ClearMem:
.loop   clr.l   (a0)+
        subq.l  #4,d0
        bgt     .loop
        rts

; Allocate block of memory using memory regions data structure.
;
; Arguments:
;   [d0] memory block size (aligned to 8 byte boundary)
;   [d1] memory flags (MB_* flags)
;   [a6] boot loader data
;
; Result:
;   [d0] allocated block of memory

AllocMem:
        movem.l d2-d5,-(sp)
        lea     BD_NREGIONS(a6),a0 ; [a0] memory regions
        move.w  (a0)+,d5        ; [d5] #regions

.lookup ; lookup suitable memory region
        movem.l (a0),d2-d3      ; [d2/d3] lower/upper address
        move.l  d3,d4
        sub.l   d2,d4           ; [d4] region size

        ; if chip memory requested upper address must be <= $200000
        btst    #MB_CHIP,d1
        beq     .nochip
        cmp.l   #CHIPMAX,d3
        bgt     .iter

.nochip ; does this region have enough space?
        cmp.l   d0,d4
        bge     .found

.iter   ; move to the next region
        add.w	#MR_SIZE,a0
        subq.l  #1,d5
        bgt     .lookup

        ; none of the regions could satisfy the request
        bra     Panic

.found  ; region found, now allocate the block!
        btst    #MB_REVERSE,d1
        bne     .rev

.fwd    ; allocate from lower address up
        add.l   d0,MR_LOWER(a0)
        move.l  d2,a1           ; [a1] allocated block
        bra     .clear

.rev    ; allocate from upper address down
        sub.l   d0,d3
        move.l  d3,MR_UPPER(a0)
        move.l  d3,a1           ; [a1] allocated block

.clear  ; check if user requested memory to be cleared
        btst    #MB_CLEAR,d1
        beq     .quit

        move.l  a1,a0
        bsr     ClearMem

.quit   move.l  a1,d0
        movem.l (sp)+,d2-d5
        rts

; Free block of memory provided it's adjacent to start / end of memory region.
;
; Arguments:
;   [d0] memory block size (aligned to 8 byte boundary)
;   [a0] start address of memory block to be freed
;   [a6] boot loader data

FreeMem:
        movem.l d2-d4,-(sp)
        add.l   a0,d0           ; [d0] end address of memory block to be freed
        lea     BD_NREGIONS(a6),a1 ; [a1] memory regions
        move.w  (a1)+,d4        ; [d4] #regions

.lookup ; lookup suitable memory region
        movem.l (a1),d2-d3      ; [d2/d3] lower/upper address

        ; block finishes at lower address ?
.lower  cmp.l   d0,d2
        bne.b   .upper
        move.l  a0,MR_LOWER(a1)
        bra.b   .quit

.upper  ; block begins at upper address ?
        cmp.l   a0,d3
        bne.b   .iter
        move.l  d0,MR_UPPER(a1)
        bra.b   .quit

.iter   ; move to the next region
        add.w	#MR_SIZE,a1
        subq.l  #1,d4
        bgt     .lookup

        ; none of the regions could satisfy the request
        bra     Panic

.quit   movem.l (sp)+,d2-d4
        rts

; Reserve correct memory type for each hunk, copy it there and relocate.
;
; Arguments:
;   [a0] executable file image
;
; Result:
;   [d0] linked list of relocated hunks

SetupHunkFile:
        ; executable amiga hunk file?
        cmp.l   #HUNK_HEADER,(a0)+
        bne     Panic

.setup  movem.l d2-d4/a2-a3,-(sp)

        ; assume there's no resident library name (skip long)
        lea     4(a0),a2

        ; read number of hunks (n)
        move.l  (a2)+,d2
        ; assume first hunk is 0 and last n-1 (skip two longs)
        addq.l  #8,a2

        ; prepare for reading hunk specifiers (size, memory type)
        lsl.l   #2,d2           ; [d2] hunk pointer array size
        sub.l   d2,sp           ; [sp] hunk pointer array

        ; allocate hunks in reverse order to make their addresses grow
        move.l  d2,d4
.alloc  move.l  -4(a2,d4.l),d0
        rol.l   #2,d0           ; [d0] hunk size, bit(1) FAST, bit(0) CHIP
        moveq.l #1,d1
        and.l   d0,d1
        and.w   #-4,d0
        addq.l  #SEG_SIZE,d0
        move.l  d0,d3
        addq.l  #MF_REVERSE|MF_CLEAR,d1
        bsr     AllocMem
        move.l  d0,-4(sp,d4.l)
        move.l  d0,a0
        move.l  d3,SEG_LEN(a0)
        cmp.l   d2,d4
        beq     .first
        move.l  (sp,d4.l),SEG_NEXT(a0)
.first  subq.l  #4,d4
        bgt     .alloc

        add.l   d2,a2           ; [a2] first hunk header

        ; parse hunks
        move.l  sp,a3
        move.l  d2,d4
.parse  move.l  (a2)+,d0        ; hunk type
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
        add.w   #SEG_START,a1
        move.l  (a2)+,d0
        rol.l   #2,d0           ; [d0] hunk specification
        moveq   #3,d1
        and.w   d0,d1           ; [d1] hunk flags
        and.w   #-4,d0          ; [d0] hunk size
        move.l  a2,a0
        add.l   d0,a2           ; move pointer to next hunk
        cmp.w   #3,d1           ; is compressed with ZX0? (HUNKF_OTHER)
        beq.b   .hunzx0
        bsr     CopyMem
        bra     .parse

.hunzx0 bsr     UnZX0
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
        addq.l  #SEG_START,d0   ; [d0] referenced hunk data address
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

;  unzx0_68000.s - ZX0 decompressor for 68000 - 88 bytes
;
;  in:  a0 = start of compressed data
;       a1 = start of decompression buffer
;
;  Copyright (C) 2021 Emmanuel Marty
;  ZX0 compression (c) 2021 Einar Saukas, https://github.com/einar-saukas/ZX0
;
;  This software is provided 'as-is', without any express or implied
;  warranty.  In no event will the authors be held liable for any damages
;  arising from the use of this software.
;
;  Permission is granted to anyone to use this software for any purpose,
;  including commercial applications, and to alter it and redistribute it
;  freely, subject to the following restrictions:
;
;  1. The origin of this software must not be misrepresented; you must not
;     claim that you wrote the original software. If you use this software
;     in a product, an acknowledgment in the product documentation would be
;     appreciated but is not required.
;  2. Altered source versions must be plainly marked as such, and must not be
;     misrepresented as being the original software.
;  3. This notice may not be removed or altered from any source distribution.

UnZX0:
               movem.l a2/d2,-(sp)  ; preserve registers
               moveq #-128,d1       ; initialize empty bit queue
                                    ; plus bit to roll into carry
               moveq #-1,d2         ; initialize rep-offset to 1

.literals:     bsr.s .get_elias     ; read number of literals to copy
               subq.l #1,d0         ; dbf will loop until d0 is -1, not 0
.copy_lits:    move.b (a0)+,(a1)+   ; copy literal byte
               dbf d0,.copy_lits    ; loop for all literal bytes
               
               add.b d1,d1          ; read 'match or rep-match' bit
               bcs.s .get_offset    ; if 1: read offset, if 0: rep-match

.rep_match:    bsr.s .get_elias     ; read match length (starts at 1)
.do_copy:      subq.l #1,d0         ; dbf will loop until d0 is -1, not 0
.do_copy_offs: move.l a1,a2         ; calculate backreference address
               add.l d2,a2          ; (dest + negative match offset)               
.copy_match:   move.b (a2)+,(a1)+   ; copy matched byte
               dbf d0,.copy_match   ; loop for all matched bytes

               add.b d1,d1          ; read 'literal or match' bit
               bcc.s .literals      ; if 0: go copy literals

.get_offset:   moveq #-2,d0         ; initialize value to $fe
               bsr.s .elias_loop    ; read high byte of match offset
               addq.b #1,d0         ; obtain negative offset high byte
               beq.s .done          ; exit if EOD marker
               move.w d0,d2         ; transfer negative high byte into d2
               lsl.w #8,d2          ; shift it to make room for low byte

               moveq #1,d0          ; initialize length value to 1
               move.b (a0)+,d2      ; read low byte of offset + 1 bit of len
               asr.l #1,d2          ; shift len bit into carry/offset in place
               bcs.s .do_copy_offs  ; if len bit is set, no need for more
               bsr.s .elias_bt      ; read rest of elias-encoded match length
               bra.s .do_copy_offs  ; go copy match

.get_elias:    moveq #1,d0          ; initialize value to 1
.elias_loop:   add.b d1,d1          ; shift bit queue, high bit into carry
               bne.s .got_bit       ; queue not empty, bits remain
               move.b (a0)+,d1      ; read 8 new bits
               addx.b d1,d1         ; shift bit queue, high bit into carry
                                    ; and shift 1 from carry into bit queue

.got_bit:      bcs.s .got_elias     ; done if control bit is 1
.elias_bt:     add.b d1,d1          ; read data bit
               addx.l d0,d0         ; shift data bit into value in d0
               bra.s .elias_loop    ; keep reading

.done:         movem.l (sp)+,a2/d2  ; restore preserved registers
.got_elias:    rts

End:

; vim: ft=asm68k:ts=8:sw=8:et
