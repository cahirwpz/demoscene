#ifndef __FIXED_POINT_H__
#define __FIXED_POINT_H__

#include "common.h"

#define SIN_MASK 0xfff
#define SIN_HALF_PI 0x400
#define SIN_PI 0x800

extern WORD sintab[];

static inline WORD SIN(WORD a) {
  a &= SIN_MASK;
  return sintab[a];
}

static inline WORD COS(WORD a) {
  a += SIN_HALF_PI;
  a &= SIN_MASK;
  return sintab[a];
}

static inline WORD normfx(LONG a) {
  asm("lsll #4,%0\n"
      "swap %0\n"
      : "+d" (a));
  return a;
}

static inline LONG shift12(WORD a) {
  LONG b;
  asm("swap %0\n"
      "clrw %0\n"
      "asrl #4,%0\n"
      : "=d" (b) : "0" (a));
  return b;
}

#define fx4i(i) \
  (WORD)((WORD)(i) << 4)

#define fx12f(f) \
  (WORD)((FLOAT)(f) * 4096.0)

__regargs LONG isqrt(LONG x);

#endif
