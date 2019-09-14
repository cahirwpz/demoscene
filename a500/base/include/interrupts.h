#ifndef __INTERRUPTS_H__
#define __INTERRUPTS_H__

#include "types.h"

#include <exec/interrupts.h>
#include <proto/exec.h>

#define INTB_SETCLR 15  /* Set/Clear control bit. Determines if bits */
                        /* written with a 1 get set or cleared. Bits */
                        /* written with a zero are allways unchanged */
#define INTB_INTEN 14   /* Master interrupt (enable only ) */
#define INTB_EXTER 13   /* External interrupt */
#define INTB_DSKSYNC 12 /* Disk re-SYNChronized */
#define INTB_RBF 11     /* serial port Receive Buffer Full */
#define INTB_AUD3 10    /* Audio channel 3 block finished */
#define INTB_AUD2 9     /* Audio channel 2 block finished */
#define INTB_AUD1 8     /* Audio channel 1 block finished */
#define INTB_AUD0 7     /* Audio channel 0 block finished */
#define INTB_BLIT 6     /* Blitter finished */
#define INTB_VERTB 5    /* start of Vertical Blank */
#define INTB_COPER 4    /* Coprocessor */
#define INTB_PORTS 3    /* I/O Ports and timers */
#define INTB_SOFTINT 2  /* software interrupt request */
#define INTB_DSKBLK 1   /* Disk Block done */
#define INTB_TBE 0      /* serial port Transmit Buffer Empty */

#define INTF_SETCLR __BIT(INTB_SETCLR)
#define INTF_INTEN __BIT(INTB_INTEN)
#define INTF_EXTER __BIT(INTB_EXTER)
#define INTF_DSKSYNC __BIT(INTB_DSKSYNC)
#define INTF_RBF __BIT(INTB_RBF)
#define INTF_AUD3 __BIT(INTB_AUD3)
#define INTF_AUD2 __BIT(INTB_AUD2)
#define INTF_AUD1 __BIT(INTB_AUD1)
#define INTF_AUD0 __BIT(INTB_AUD0)
#define INTF_BLIT __BIT(INTB_BLIT)
#define INTF_VERTB __BIT(INTB_VERTB)
#define INTF_COPER __BIT(INTB_COPER)
#define INTF_PORTS __BIT(INTB_PORTS)
#define INTF_SOFTINT __BIT(INTB_SOFTINT)
#define INTF_DSKBLK __BIT(INTB_DSKBLK)
#define INTF_TBE __BIT(INTB_TBE)

#define INTERRUPT(NAME, PRI, HANDLER, DATA)             \
static struct Interrupt *NAME = &(struct Interrupt) {   \
  { NULL, NULL, NT_INTERRUPT, PRI, (char *)#NAME },     \
  (void *)DATA, (void *)&HANDLER                        \
};

void DumpInterrupts(void);

/* Following functions expect INTF_* constants! */
void EnableINT(u_short mask);
void DisableINT(u_short mask);
void ClearIRQ(u_short mask);
void WaitIRQ(u_short mask);

#endif
