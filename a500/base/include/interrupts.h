#ifndef __INTERRUPTS_H__
#define __INTERRUPTS_H__

#include <exec/interrupts.h>
#include <hardware/intbits.h>
#include <proto/exec.h>

#include "hardware.h"

#define INTERRUPT(NAME, PRI, HANDLER, DATA)             \
static struct Interrupt *NAME = &(struct Interrupt) {   \
  { NULL, NULL, NT_INTERRUPT, PRI, #NAME },             \
  (APTR)DATA, (APTR)&HANDLER                            \
};

void DumpInterrupts();

/* Following functions expect INTF_* constants! */
void EnableINT(UWORD mask);
void DisableINT(UWORD mask);
void ClearIRQ(UWORD mask);
void WaitIRQ(UWORD mask);

#endif
