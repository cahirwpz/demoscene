#ifndef __STD_FP16_H__
#define __STD_FP16_H__

#include "std/types.h"

typedef struct {
  int16_t integer;
  uint16_t fraction;
} Q16T;

Q16T CastFloatQ16(float value asm("fp0"));
Q16T CastIntQ16(int value asm("d0"));

static inline void IAddQ16(Q16T *result asm("a0"), Q16T value asm("d0")) {
  (*(int32_t *)result) += (*(int32_t *)&value);
}

static inline Q16T AddQ16(Q16T a asm("d0"), Q16T b asm("d1")) {
  int32_t c = (*(int32_t *)&a) + (*(int32_t *)&b);
  return *(Q16T *)&c;
}

Q16T *CalcSineTableQ16(size_t n asm("d0"), size_t frequency asm("d1"),
                       float amplitude asm("fp0"), float shift asm("fp1"));

#endif
