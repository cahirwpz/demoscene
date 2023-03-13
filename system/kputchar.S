#include <asm.h>
#include <custom_regdef.h>

# [a0.l] custom
# [d0.b] character
ENTRY(KPutChar)
        tst.b   d0              /* don't send character if it's zero */
        beq.s   .Quit

        move.w  d0,-(sp)
        cmp.b   #10,d0          /* send '\r' first if the character is `\n` */
        bne.s   .NoNL
        moveq   #13,d0
        bsr.s   .Send
.NoNL:  move.w  (sp)+,d0

.Send:  move.w  #0x100,d1       /* add stop bit */
        move.b  d0,d1

.Wait:  btst.b  #5,serdatr(a0)  /* loop until transmit buffer empty */
        beq.s   .Wait
        move.w  d1,serdat(a0)   /* send the character */

.Quit:  rts
END(KPutChar)

# vim: ft=gas:ts=8:sw=8:noet:
