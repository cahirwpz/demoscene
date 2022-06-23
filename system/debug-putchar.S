# Reliable transfers may only happen 3 or more E-cycles basis.
# http://lallafa.de/blog/2015/09/amiga-parallel-port-how-fast-can-you-go/

#include <asm.h>
#include <cia_regdef.h>

# [a0] ciab, ciaa = ciab + $1001
ENTRY(DPutChar)
.Loop:  btst    #CIAB_PRTRBUSY,(a0)     /* is printer buffer empty ? */
        jne     .Loop

        st.b    ciaddrb+0x1001(a0)      /* set direction to out */
        move.b  d0,ciaprb+0x1001(a0)	/* write the data */
        tst.b   (a0)                    /* waste 2 E-cycles */
        tst.b   (a0)
        rts
END(DPutChar)

# vim: ft=gas:ts=8:sw=8:noet:
