#include <asm.h>

ENTRY(start)
#ifdef AMIGAOS
        jsr     _L(SaveOS)
        move.l  d0, a0
        jsr     _L(Loader)
        jsr     _L(RestoreOS)
        rts
#else
        jsr     _L(Loader)
        jra     .
#endif
END(start)

# vim: ft=gas:ts=8:sw=8:noet
