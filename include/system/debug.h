#ifndef __SYSTEM_DEBUG_H__
#define __SYSTEM_DEBUG_H__

#include <config.h>
#include <cdefs.h>
#include <types.h>

struct BootData;

void CrashInit(struct BootData *bd);
__noreturn void Crash(void);

#if DEBUG == 0
#define Assert(e) ((void)(e))
#define Log(...) ((void)0)
#define Panic(...) Crash()
#define HexDump(ptr, len) { (void)(ptr); (void)(len); }
#endif

#if DEBUG >= 1
#define Assert(e) { if (!(e))                                                  \
  Panic("Assertion \"%s\" failed: file \"%s\", line %d, function \"%s\"!",     \
        #e, __FILE__, __LINE__, __func__); }

#ifdef UAE
#include <uae.h>
#define Log(...) UaeLog(__VA_ARGS__);
#define Panic(...) { UaeLog(__VA_ARGS__); BREAK(); Crash(); }
#define HexDump(...) ((void)0) /* TODO */
#else
void Log(const char *format, ...)
  __attribute__ ((format (printf, 1, 2)));
__noreturn void Panic(const char *format, ...)
  __attribute__ ((format (printf, 1, 2)));
void HexDump(const void *ptr, u_int len);
#endif
#endif

#ifdef _SYSTEM
#if DEBUG >= 2
#define Debug(fmt, ...) Log("[%s] " fmt "\n", __func__, __VA_ARGS__)
#define Assume(e) Assert(e)
#else
#define Debug(fmt, ...) ((void)0)
#define Assume(e) { if (!(e)) HALT(); }
#endif
#endif /* !_SYSTEM */

#endif /* !__SYSTEM_DEBUG_H__ */
