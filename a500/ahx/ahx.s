; vim: ft=asm68k:ts=8:sw=8:

        xdef _AhxInitHardware
        xdef _AhxInitPlayer
        xdef _AhxInitModule
        xdef _AhxInitSubSong
        xdef _AhxInterrupt
        xdef _AhxStopSong
        xdef _AhxKillPlayer
        xdef _AhxKillHardware
        xdef _AhxNextPattern
        xdef _AhxPrevPattern
        xdef _Ahx

        xref _MemAlloc
        xref _MemFree
        xref _OpenFile
        xref _FileRead
        xref _CloseFile

	section	ahx,Code

Ahx:
        incbin 'AHX-Replayer000.BIN.patched'

        cargs #Ahx+$28, _Ahx
        cargs #Ahx+$2c, _AhxChip
        cargs #Ahx+$30, _AhxSize
        cargs #Ahx+$34, _AhxChipSize
        cargs #Ahx+$38, _Module
        cargs #Ahx+$70, __AhxInitCIA
        cargs #Ahx+$194, __AhxKillCIA
        cargs #Ahx+$288, _AhxKillPlayer
        cargs #Ahx+$368, __AhxInitPlayer
        cargs #Ahx+$73a, __AhxInitHW
        cargs #Ahx+$828, _AhxInitModule
        cargs #Ahx+$902, _AhxInitSubSong
        cargs #Ahx+$992, _AhxStopSong
        cargs #Ahx+$9bc, _AhxNextPattern
        cargs #Ahx+$9d0, _AhxPrevPattern
        cargs #Ahx+$9ea, _AhxInterrupt

jumptable:
        bra.w   _AllocMem
        bra.w   _FreeMem
        bra.w   _Open
        bra.w   _Read
        bra.w   _Close

_AllocMem:
        jmp     _MemAlloc

_FreeMem:
        jmp     _MemFree

_Open:
        clr.l   d0
        jmp     _OpenFile

_Read:
        move.l  d1,a0
        jsr     _FileRead
        neg.l   d0
        and.l   d3,d0
        rts

_Close:
        move.l  d1,a0
        jmp     _CloseFile

_AhxInitHW:
	movem.l	d1-a6,-(sp)
        move.l  _Ahx+4(pc),a5
        bra     __AhxInitHW

_AhxInitHardware:
        bsr     _AhxInitHW

        movem.l d6/a2-a4/a6,-(sp)
        bsr     __AhxInitCIA
        movem.l (sp)+,d6/a2-a4/a6
        rts

_AhxInitPlayer:
        suba.l  a0,a0
        suba.l  a1,a1
        jmp     __AhxInitPlayer

_AhxKillHardware:
        movem.l a3/a4/a6,-(sp)
        bsr     __AhxKillCIA
        movem.l (sp)+,a3/a4/a6
        rts
