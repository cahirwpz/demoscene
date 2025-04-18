#include <asm.h>

ENTRY(UserState34)
        movea.l (sp)+,a1
        move.l  sp,usp
        movea.l a0,sp
        and.w   #0xdfff,sr
        jmp     (a1)
END(UserState34)

# vim: ft=gas:ts=8:sw=8:noet:
