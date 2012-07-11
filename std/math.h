#ifndef __STD_MATH__
#define __STD_MATH__

#include <math.h>

#ifdef AMIGA
inline static float truncf(float x) {
  float value;

  asm("fintrz%.x %1,%0"
      : "=f" (value)
      : "f" (x));

  return value;
}

inline static int lroundf(float x) {
  float real;
  int value;

  asm("fintrz%.x %1,%0"
      : "=f" (real)
      : "f" (x + 0.5f));
  asm("fmove%.l %1,%0"
      : "=d" (value)
      : "f" (real));

  return value;
}

inline static float modff(float x, float *ip) {
  float temp;

  asm("fintrz%.x %1,%0"
      : "=f" (temp)
      : "f" (x));

  *ip = temp;

  return x - temp;
}
#endif

#endif
