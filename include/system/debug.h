#ifndef __SYSTEM_DEBUG_H__
#define __SYSTEM_DEBUG_H__

#include <cdefs.h>

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

#endif /* !__SYSTEM_DEBUG__ */
