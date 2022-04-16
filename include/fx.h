#ifndef __FIXED_POINT_H__
#define __FIXED_POINT_H__

#include "common.h"

#define SIN_MASK 0xfff
#define SIN_HALF_PI 0x400
#define SIN_PI 0x800

extern short sintab[];

static inline short SIN(short a) {
  return getword(sintab, a & SIN_MASK);
}

static inline short COS(short a) {
  return getword(sintab, (a + SIN_HALF_PI) & SIN_MASK);
}

static inline short normfx(int a) {
  asm("lsll #4,%0\n\t"
      "swap %0"
      : "+d" (a));
  return a;
}

static inline int shift12(short a) {
  int b;
  asm("swap %0\n\t"
      "clrw %0\n\t"
      "asrl #4,%0"
      : "=d" (b) : "0" (a));
  return b;
}

#define fx4i(i) \
  (short)((u_short)(i) << 4)

#define fx12f(f) \
  (short)((float)(f) * 4096.0)

int isqrt(int x);

#endif
