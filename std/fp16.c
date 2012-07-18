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

int IntRoundQ16(Q16T value asm("d0")) {
  int v = AsInt(value);

  if (v < 0)
    v -= 0x7fff;
  else
    v += 0x8000;

  return v / 65536;
}

Q16T ReciprocalIntQ16(int value asm("d0")) {
  Q16T result = { 0, 0 };

  if (value > 1)
    result.fraction = 0x10000 / value;
  else if (value == 1)
    result.integer = 1;

  /* don't handle reciprocal of zero */

  return result;
}

Q16T *CalcSineTableQ16(size_t n asm("d0"), size_t frequency asm("d1"),
                       float amplitude asm("fp0"), float shift asm("fp1"))
{
  Q16T *table = NewTable(Q16T, n);
  float iter = 2 * (float)M_PI * shift;
  float step = 2 * (float)M_PI * (int)frequency / (int)n;
  size_t i;

  for (i = 0; i < n; i++, iter += step)
    table[i] = CastFloatQ16(sin(iter) * amplitude);

  return table;
}
