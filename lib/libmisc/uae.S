#include <asm.h>

ENTRY(_UaeTrap)
        jmp     0xf0ff60
END(_UaeTrap)

STRONG_ALIAS(_UaeLog, _UaeTrap)
STRONG_ALIAS(_UaeWarpMode, _UaeTrap)

# vim: ft=gas:ts=8:sw=8:noet:
