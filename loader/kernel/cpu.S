#include <asm.h>

ENTRY(SetIPL)
        move.w  sr,-(sp)
        move.w  sr,d1
        and.w   #0xf8ff,d1              /* old SR with IPL bits cleared */
        and.w   #0x0700,d0              /* argument with IPL bits left */
        or.w    d0,d1
        move.w  d1,sr                   /* set new value of IPL in SR */
        move.w  (sp)+,d0
        and.w   #0x0700,d0              /* return IPL bits of original SR */
        rts
END(SetIPL)

# vim: ft=gas:ts=8:sw=8:noet
