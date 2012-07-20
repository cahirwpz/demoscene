#ifndef __STD_MATH__
#define __STD_MATH__

#define __HAVE_68881__

#include <math.h>
#include "std/types.h"

#ifdef AMIGA
inline static float log2f(float x) {
  float value;

  asm("flog2%.x %1,%0"
      : "=f" (value)
      : "f" (x));

  return value;
}

inline static double log2(double x) {
  double value;

  asm("flog2%.x %1,%0"
      : "=f" (value)
      : "f" (x));

  return value;
}

inline static float logf(float x) {
  float value;

  asm("flogn%.x %1,%0"
      : "=f" (value)
      : "f" (x));

  return value;
}

inline static float sqrtf(float x) {
  float value;

  asm("fsqrt%.x %1,%0"
      : "=f" (value)
      : "f" (x));

  return value;
}

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

float *CalcSineTable(size_t n asm("d0"), size_t frequency asm("d1"),
                     float amplitude asm("fp0"), float shift asm("fp1"));

#endif
