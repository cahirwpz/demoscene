#include <asm.h>

# [a0] jmp_buf
# [d0] return val
ENTRY(longjmp)
        movem.l (a0)+,d2-d7/a2-a7       /* restore caller registers */
        move.l  (a0),(sp)               /* restore saved pc */
        rts
END(longjmp)

# vim: ft=gas:ts=8:sw=8:noet:
