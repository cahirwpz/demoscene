#include <asm.h>

# [a0] jmp_buf
ENTRY(setjmp)
        movem.l d2-d7/a2-a7,(a0)        /* save caller registers */
        move.l  (sp),48(a0)             /* and return address */
        moveq.l #0,d0                   /* return 0 */
        rts
END(setjmp)

# vim: ft=gas:ts=8:sw=8:noet:
