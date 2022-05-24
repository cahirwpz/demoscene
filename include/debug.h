#ifndef __DEBUG_H__
#define __DEBUG_H__

/* When simulator is configured to enter debugger on illegal instructions,
 * this macro can be used to set breakpoints in your code. */
#define BREAK() { asm volatile("illegal"); }

/* Halt the processor by masking all interrupts and waiting for NMI. */
#define HALT() { asm volatile("stop #0x2700"); }

/* Use whenever a program should generate a fatal error. This will break into
 * debugger for program inspection and stop instruction execution. */
#define PANIC() { BREAK(); HALT(); }

#include <system/debug.h>

#ifdef NDEBUG
#define Assert(e) {}
#else
#define Assert(e) { if (!(e))                                                  \
   Panic("Assertion \"%s\" failed: file \"%s\", line %d, function \"%s\"!\n",  \
         #e, __FILE__, __LINE__, __func__); }
#endif

#endif 
