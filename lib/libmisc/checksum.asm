        xdef    _Checksum

        section '.text',code

_Checksum:
        subq.l  #1,d1
        asr.l   #2,d1

.loop:  add.l   (a0)+,d0
        dbf     d1,.loop

        swap    d1
        subq.w  #1,d1
        bmi.s   .exit
        swap    d1
        bra.s   .loop

.exit:
        rts

; vim: ft=asm68k:ts=8:sw=8:
