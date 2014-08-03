#ifndef __FIXED_POINT_H__
#define __FIXED_POINT_H__

#include "common.h"

#define SINCOS_MASK 0xfff

extern SinCosT sincos[];

static inline WORD normfx(LONG a) {
  asm("lsll #4,%0\n"
      "swap %0\n"
      : "+d" (a));
  return a;
}

#define fx4i(i) \
  (WORD)((WORD)(i) << 4)

#define fx12f(f) \
  (WORD)((FLOAT)(f) * 4096.0)

#endif
