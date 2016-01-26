#ifndef __INTERRUPTS_H__
#define __INTERRUPTS_H__

#include <exec/interrupts.h>
#include <hardware/intbits.h>
#include <proto/exec.h>

#define INTERRUPT(NAME, PRI, HANDLER)       \
static struct Interrupt NAME = {            \
  { NULL, NULL, NT_INTERRUPT, PRI, #NAME }, \
  NULL, (APTR)&HANDLER                      \
};

void DumpInterrupts();

#endif
