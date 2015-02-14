#include "std/memory.h"
#include "std/math.h"

float *CalcSineTable(size_t n asm("d0"), size_t frequency asm("d1"),
                     float amplitude asm("fp0"), float shift asm("fp1"))
{
  float *table = NewTable(float, n);
  float iter = 2 * (float)M_PI * shift;
  float step = 2 * (float)M_PI * (int)frequency / (int)n;
  size_t i;

  for (i = 0; i < n; i++, iter += step)
    table[i] = sin(iter) * amplitude;

  return table;
}
