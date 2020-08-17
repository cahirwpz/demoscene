#include <asm.h>

ENTRY(start)
        jsr     _L(Loader)
        jra     .
END(start)

# vim: ft=gas:ts=8:sw=8:noet
