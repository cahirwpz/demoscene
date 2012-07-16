#include "std/fp16.h"
#include "std/math.h"

void CastFloatQ16(Q16T *result asm("a0"), float value asm("fp0")) {
  float fraction;
  float integer = modff(value, &fraction);

  result->integer = lroundf(integer);
  result->fraction = lroundf(fraction * 65536);
}

void CastIntQ16(Q16T *result asm("a0"), int value asm("d0")) {
  result->integer = value;
  result->fraction = 0;
}

void IAddQ16(Q16T *result asm("a0"), Q16T value asm("d0")) {
  uint32_t *a = (uint32_t *)&value;
  uint32_t *b = (uint32_t *)result;

  (*b) += (*a);
}
