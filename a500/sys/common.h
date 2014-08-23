#ifndef __COMMON_H__
#define __COMMON_H__

#include <exec/types.h>

#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))
#define abs(a) (((a) < 0) ? (-(a)) : (a))

#define offsetof(st, m) \
  ((ULONG)((char *)&((st *)0)->m - (char *)0))

static inline WORD absw(WORD a) {
  if (a < 0)
    return -a;
  return a;
}

static inline ULONG swap16(ULONG a) {
  asm("swap %0": "=d" (a));
  return a;
}

static inline UWORD swap8(UWORD a) {
  return (a << 8) | (a >> 8);
}

static inline WORD div16(LONG a, WORD b) {
  asm("divs %1,%0"
      : "+d" (a)
      : "d" (b));
  return a;
}

static inline void bset(UBYTE *ptr, WORD bit) {
  asm("bset %1,%0" :: "m" (*ptr), "d" (bit));
}

#define rorw(a, b) \
  (((a) << (16 - (b))) | ((a) >> (b)))

#define swapr(a, b) \
  asm ("exg %0,%1" : "+r" (a), "+r" (b))

#define pushl(a) \
  asm ("movel %0,%-" :: "r" (a))

#define popl(a) \
  asm ("movel %+,%0" : "=r" (a))

void Log(const char *format, ...) __attribute__ ((format (printf, 1, 2)));

#endif
