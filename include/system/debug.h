#ifndef __SYSTEM_DEBUG_H__
#define __SYSTEM_DEBUG_H__

#include <config.h>
#include <cdefs.h>

#ifdef UAE
#include <uae.h>
#define Log(...) UaeLog(__VA_ARGS__);
#define Panic(...) { UaeLog(__VA_ARGS__); PANIC(); }
#else
#if LOGOUT > 0
void Log(const char *format, ...)
  __attribute__ ((format (printf, 1, 2)));
__noreturn void Panic(const char *format, ...)
  __attribute__ ((format (printf, 1, 2)));
#else
#define Log(...) ((void)0)
#define Panic(...) HALT()
#endif
#endif

#ifdef _SYSTEM
#ifdef DEBUG
#define Debug(fmt, ...) Log("[%s] " fmt "\n", __func__, __VA_ARGS__)
#define Assume(e) Assert(e)
#else
#define Debug(fmt, ...) ((void)0)
#define Assume(e) { if (!(e)) HALT(); }
#endif
#endif /* !_SYSTEM */

#endif /* !__SYSTEM_DEBUG_H__ */
