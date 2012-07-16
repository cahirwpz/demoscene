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

void IAddQ16(Q16T *result asm("a0"), Q16T value asm("d0")) {
  int32_t *a = (int32_t *)&value;
  int32_t *b = (int32_t *)result;

  (*b) += (*a);
}
