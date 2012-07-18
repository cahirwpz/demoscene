#include "std/fp16.h"
#include "std/math.h"
#include "std/memory.h"

Q16T CastFloatQ16(float value asm("fp0")) {
  float fraction;
  float integer = modff(value, &fraction);
  Q16T result = { lroundf(integer), lroundf(fraction * 65536) };
  return result;
}

Q16T CastIntQ16(int value asm("d0")) {
  Q16T result = { value, 0 };
  return result;
}

Q16T *CalcSineTableQ16(size_t n asm("d0"), WaveDescT *sine asm("a0")) {
  Q16T *table = NewTable(Q16T, n);
  float iter = 2 * (float)M_PI * sine->shift;
  float step = 2 * (float)M_PI * (int)sine->frequency / (int)n;
  size_t i;

  for (i = 0; i < n; i++, iter += step)
    table[i] = CastFloatQ16(sin(iter) * sine->amplitude);

  return table;
}
