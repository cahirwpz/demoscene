# Reliable transfers may only happen 3 or more E-cycles basis.
# http://lallafa.de/blog/2015/09/amiga-parallel-port-how-fast-can-you-go/

#include <asm.h>
#include <cia_regdef.h>

/* It's a reimplementation of PRawPutChar in ddebug.lib from NDK3.2 */

# [a0.l] ciab, ciaa = ciab + $1001
# [d0.b] character
ENTRY(DPutChar)
        tst.b   d0              /* don't send character if it's zero */
        beq.s   .Quit

        move.w  d0,-(sp)
        cmp.b   #10,d0          /* send '\r' first if the character is `\n` */
        bne.s   .NoNL
        moveq   #13,d0
        bsr.s   .Send
        tst.b   (a0)                    /* waste single E-cycle */
.NoNL:  move.w  (sp)+,d0

.Send:  tst.b   (a0)                    /* waste 2 E-cycles */
        tst.b   (a0)
.Wait:  btst    #CIAB_PRTRBUSY,(a0)     /* is printer buffer empty ? */
        bne.s   .Wait

        st.b    ciaddrb+0x1001(a0)      /* set direction to out */
        move.b  d0,ciaprb+0x1001(a0)	/* write the data */
        tst.b   (a0)                    /* waste another 2 E-cycles */
        tst.b   (a0)

.Quit:  rts
END(DPutChar)

# vim: ft=gas:ts=8:sw=8:noet:
