#include <asm.h>

ENTRY(GetVBR)
        movec   vbr,d0
        rte
END(GetVBR)

# vim: ft=gas:ts=8:sw=8:noet
