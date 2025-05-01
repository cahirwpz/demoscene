*****************************************************************
*                                                               *
* VBRMove version 2.3, © 1991 Arthur Hagen, All rights reserved *
*                                                               *
* Posted to the Public Domain.                                  *
*                                                               *
* Written using the Aztec Manx C 5.0 Assembler                  *
*                                                               *
* All comments deliberately stripped from source.               *
* If you don't understand it, you should not mess with it!      *
*                                                               *
*****************************************************************

        include 'exec/memory.i'
        include 'exec/execbase.i'
        include 'lvo/exec_lib.i'
        include 'lvo/dos_lib.i'

INTENA  equ     $dff09a

        machine mc68010
        section code

_VBRMove:
        movea.l (4).w,a6
        lea     INTENA,a3
        moveq   #64,d6
        rol.w   #4,d6
        lea     dosname(pc),a1
        jsr     _LVOOldOpenLibrary(a6)
        movea.l d0,a5
        exg.l   a5,a6
        jsr     _LVOOutput(a6)
        exg.l   a5,a6
        move.l  d0,d7
        moveq   #title_end-title,d3
        lea     title(pc),a0
        bsr     wrt
        btst    #AFB_68010,AttnFlags+1(a6)
        bne.s   tryit

badCPU  moveq   #wrongpro_end-wrongpro,d3
        lea     wrongpro(pc),a0
        bra.w   writerr

tryit   jsr     _LVOSuperState(a6)
        movec   VBR,d4
        move.l  sp,USP
        movea.l d0,sp
        andi.w  #$dfff,SR
        tst.l   d4
        bne.w   alrdy

        lea     MemList+LH_HEAD(a6),a0
        moveq   #0,d2
        moveq   #$3f,d3
        not.b   d3
        moveq   #0,d5
        not.b   d5

loopit1 movea.l LN_SUCC(a0),a0
        tst.l   LN_SUCC(a0)
        beq.s   memdon1

        move.w  MH_LOWER(a0),d0

        cmp.w   d3,d0
        bcc.s   loopit1
        cmp.w   d5,d0
        bcs.s   loopit1

        lea     MH_FIRST(a0),a4
        lea     MH_FREE(a0),a2
        move.w  #$4000,(a3)
        addq.b  #1,IDNestCnt(a6)
        move.l  (a4),d2
        move.l  (a2),d3
        moveq   #0,d0
        move.l  d0,(a4)
        move.l  d0,(a2)

memdon1 move.l  d6,d0
        addq.l  #8,d0
        moveq   #MEMF_FAST|MEMF_PUBLIC,d1
        jsr     _LVOAllocMem(a6)
        tst.l   d2
        beq.s   memdon2

        move.l  d2,(a4)
        move.l  d3,(a2)
        subq.b  #1,IDNestCnt(a6)
        bge.s   memdon2
        move.w  #$c000,(a3)

memdon2 move.l  d0,d4
        beq.s   nofm

        andi.w  #$fff0,d4
        suba.l  a0,a0
        movea.l d4,a1
        bsr.s   copy1k
        move.l  d4,d3
        bsr.s   setvbr

quit    movea.l a5,a1
        jmp     _LVOCloseLibrary(a6)

nofm    moveq   #nofastmem_end-nofastmem,d3
        lea     nofastmem(pc),a0

writerr pea     quit(pc)

wrt     move.l  a0,d2
        move.l  d7,d1
        exg.l   a5,a6
        jsr     _LVOWrite(a6)
        exg.l   a5,a6
        rts

alrdy   moveq   #already_end-already,d3
        lea     already(pc),a0
        bsr.s   wrt
        move.l  d4,a0
        suba.l  a1,a1
        bsr.s   copy1k
        moveq   #0,d3
        bsr.s   setvbr
        move.l  d4,a1
        move.l  d6,d0
        jsr     _LVOFreeMem(a6)
        bra.s   quit

copy1k  move.l  d6,d0
        jmp     _LVOCopyMemQuick(a6)

setvbr  jsr     _LVOSuperState(a6)
        movec   d3,VBR
        move.l  sp,USP
        movea.l d0,sp
        andi.w  #$dfff,SR
        rts

        dc.b    '$VER: '
title   dc.b    'VBRMove 2.3 (24.4.94)',$0A,$0D
        dc.b    'Copyright ',$A9,' 1992 Arthur Hagen.',$0A
title_end
already dc.b    'Clearing VBR.',$0A
already_end
wrongpro
        dc.b    'You need 68010 or higher!',$0A
wrongpro_end
nofastmem
        dc.b    "No fast memory!",$0A
nofastmem_end
dosname dc.b    'dos.library',0
        even
