; vim: ft=asm68k:ts=8:sw=8:

        xdef _AhxInitCIA
        xdef _AhxInitPlayer
        xdef _AhxInitModule
        xdef _AhxInitSubSong
        xdef _AhxInterrupt
        xdef _AhxStopSong
        xdef _AhxKillPlayer
        xdef _AhxKillCIA
        xdef _AhxNextPattern
        xdef _AhxPrevPattern
        xdef _Ahx

	section	ahx,Code

Ahx:
        incbin  AHX-Replayer000.BIN
;       incbin  AHX-Replayer020.BIN

        cargs #Ahx+0, AhxInitCIA
        cargs #Ahx+4, _AhxInitPlayer
        cargs #Ahx+8, _AhxInitModule
        cargs #Ahx+12, _AhxInitSubSong
        cargs #Ahx+16, _AhxInterrupt
        cargs #Ahx+20, _AhxStopSong
        cargs #Ahx+24, _AhxKillPlayer
        cargs #Ahx+28, AhxKillCIA
        cargs #Ahx+32, _AhxNextPattern
        cargs #Ahx+36, _AhxPrevPattern
        cargs #Ahx+40, _Ahx

_AhxInitCIA:
        movem.l d6/a2-a4/a6,-(sp)
        bsr     AhxInitCIA
        movem.l (sp)+,d6/a2-a4/a6
        rts

_AhxKillCIA:
        movem.l a3/a4/a6,-(sp)
        bsr     AhxKillCIA
        movem.l (sp)+,a3/a4/a6
        rts
