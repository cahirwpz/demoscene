#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <cdefs.h>

/* When simulator is configured to enter debugger on illegal instructions,
 * this macro can be used to set breakpoints in your code. */
#define BREAK() { asm volatile("\tillegal\n"); }

/* Halt the processor by masking all interrupts and waiting for NMI. */
#define HALT() { asm volatile("\tstop\t#0x2700\n"); }

/* Use whenever a program should generate a fatal error. This will break into
 * debugger for program inspection and stop instruction execution. */
#define PANIC() { BREAK(); HALT(); }

#ifdef UAE
#include <system/uae.h>
#define Log(...) UaeLog(__VA_ARGS__);
#define Panic(...) { UaeLog(__VA_ARGS__); PANIC(); }
#else
void Log(const char *format, ...)
  __attribute__ ((format (printf, 1, 2)));
__noreturn void Panic(const char *format, ...)
  __attribute__ ((format (printf, 1, 2)));
#endif

#ifdef NDEBUG
#define Assert(e) {}
#else
#define Assert(e) { if (!(e))                                                  \
   Panic("Assertion \"%s\" failed: file \"%s\", line %d, function \"%s\"!\n",  \
         #e, __FILE__, __LINE__, __func__); }
#endif

#endif 
