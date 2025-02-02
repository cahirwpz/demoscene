#ifndef __DEBUG_H__
#define __DEBUG_H__

/* When simulator is configured to enter debugger on illegal instructions,
 * this macro can be used to set breakpoints in your code. */
#define BREAK() { asm volatile("illegal"); }

/* Halt the processor by masking all interrupts and waiting for NMI. */
#define HALT() { asm volatile("stop #0x2700"); }

#include <system/debug.h>

#endif
