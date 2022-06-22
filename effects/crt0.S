#include <asm.h>

ENTRY(start)
        move.l  #_L(Effect),d0
        rts
END(start)

#include "../system/syscall.S"

# vim: ft=gas:ts=8:sw=8:noet
