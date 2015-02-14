#include "std/fp16.h"
#include "std/math.h"
#include "std/memory.h"

__regargs FP16 *
CalcSineTableFP16(size_t n, size_t frequency, float amplitude, float shift) {
  FP16 *table = NewTable(FP16, n);
  float iter = 2 * (float)M_PI * shift;
  float step = 2 * (float)M_PI * (int)frequency / (int)n;
  size_t i;

  for (i = 0; i < n; i++, iter += step)
    table[i] = FP16_float(sin(iter) * amplitude);

  return table;
}
