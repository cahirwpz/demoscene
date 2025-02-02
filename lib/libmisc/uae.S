#include <config.h>
#include <asm.h>

ENTRY(_UaeTrap)
#ifdef UAE
	/* illegal + rts */
	cmp.l   #0xa0134e75,0xf0ff60
	beq     .Luae
	rts
.Luae:  jmp     0xf0ff60
#else
	rts
#endif
END(_UaeTrap)

STRONG_ALIAS(_UaeLog, _UaeTrap)
STRONG_ALIAS(_UaeWarpMode, _UaeTrap)

# vim: ft=gas:ts=8:sw=8:noet:
