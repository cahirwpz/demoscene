        xdef _GetContext
        xdef _SetContext
        xdef _SwapContext

oDREGS  equ 4*0
oAREGS  equ 4*8
oSP     equ 4*15
oPC     equ 4*16
oSR     equ 4*17

	section	text,code

_GetContext:
        movem.l d0-d7,oDREGS(a0)
        movem.l a0-a6,oAREGS(a0)
        move.l  (sp),oPC(a0)
        lea     4(sp),sp
        move.l  sp,oSP(a0)
        lea     -4(sp),sp
        rts

_SetContext:
        movem.l oDREGS(a0),d0-d7
        movem.l oAREGS+2*4(a0),a2-a6/sp
        move.l  oPC(a0),-(sp)
        movem.l oAREGS(a0),a0-a1
        rts

_SwapContext:
        movem.l d0-d7,oDREGS(a0)
        movem.l a0-a6,oAREGS(a0)
        move.l  (sp),oPC(a0)
        lea     4(sp),sp
        move.l  sp,oSP(a0)

        movem.l oDREGS(a1),d0-d7
        movem.l oAREGS+2*4(a1),a2-a6/sp
        move.l  oPC(a1),-(sp)
        movem.l oAREGS(a0),a0-a1
        rts

; vim: ft=asm68k:ts=8:sw=8
