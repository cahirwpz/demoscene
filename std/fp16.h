#ifndef __STD_FP16_H__
#define __STD_FP16_H__

#include "std/types.h"

#define AsInt(a) (*(int *)&(a))
#define AsQ16(a) (*(Q16T *)&(a))

typedef struct {
  int16_t integer;
  uint16_t fraction;
} Q16T;

static inline Q16T CastFloatQ16(float value) {
  int integer = (int)(value * 65536);
  return AsQ16(integer);
}

static inline Q16T CastIntQ16(int value) {
  Q16T result = { value, 0 };
  return result;
}

int IntRoundQ16(Q16T value asm("d0"));

static inline void IAddQ16(Q16T *result, Q16T value) {
  (*(int *)result) += AsInt(value);
}

static inline Q16T AddQ16(Q16T a, Q16T b) {
  int c = AsInt(a) + AsInt(b);
  return AsQ16(c);
}

static inline Q16T SubQ16(Q16T a, Q16T b) {
  int c = AsInt(a) - AsInt(b);
  return AsQ16(c);
}

Q16T ReciprocalIntQ16(int value asm("d0"));

Q16T *CalcSineTableQ16(size_t n asm("d0"), size_t frequency asm("d1"),
                       float amplitude asm("fp0"), float shift asm("fp1"));

#endif
