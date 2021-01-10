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
        xref _FileClose

	section	'.text',code

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
        cargs #Ahx+$1650, _AhxSetupVoice

jumptable:
        bra.w   _AllocMem               ; 2d3c
        bra.w   _FreeMem                ; 2d40
        bra.w   _Open                   ; 2d44
        bra.w   _Read                   ; 2d48
        bra.w   _Close                  ; 2d4c
        bra.w   _OldOpenLibrary         ; 2d50
        bra.w   _CloseLibrary           ; 2d54
        bra.w   _OpenResource           ; 2d58
        bra.w   _AddICRVector           ; 2d5c
        bra.w   _RemICRVector           ; 2d60
        bra.w   _Cause                  ; 2d64

; void *AllocMem(ULONG byteSize asm("d0"), ULONG attributes asm("d1"))
; -> void *MemAlloc(u_int byteSize asm("d0"), u_int attributes asm("d1"))
_AllocMem:
        jmp     _MemAlloc

; void FreeMem(void *memoryBlock asm("a1"), ULONG byteSize asm("d0"))
; -> void MemFree(void *memoryBlock asm("a0")
_FreeMem:
        move.l  a1,a0
        jmp     _MemFree

; BPTR Open(STRPTR name asm("d1"), LONG accessMode asm("d2"))
; -> FileT *OpenFile(const char *path asm("a0"));
_Open:
        move.l  d1,a0
        jmp     _OpenFile

; LONG Read(BPTR file asm("d1"), void *buffer asm("d2"), LONG length asm("d3"))
; -> int FileRead(FileT *f asm("a0"), void *buf asm("a1"), u_int nbyte asm("d0")
_Read:
        move.l  d1,a0
        move.l  d2,a1
        move.l  d3,d0
        jmp     _FileRead

; BOOL Close(BPTR file asm("d1"))
; -> void FileClose(FileT *f asm("a0"))
_Close:
        move.l  d1,a0
        jmp     _FileClose

; OldOpenLibrary
_OldOpenLibrary:
        moveq.l #1,d0
        rts

; CloseLibrary
_CloseLibrary:
        moveq.l #1,d0
        rts

; OpenResource
_OpenResource:
        moveq.l #0,d0
        rts

; AddICRVector
_AddICRVector:
        rts

; RemICRVector
_RemICRVector:
        rts

; Cause
_Cause:
        rts

; Routines exported to C
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
        bra     __AhxInitPlayer

_AhxKillHardware:
        movem.l a3/a4/a6,-(sp)
        bsr     __AhxKillCIA
        movem.l (sp)+,a3/a4/a6
        rts
