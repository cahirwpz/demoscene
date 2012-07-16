#ifndef __STD_FP16_H__
#define __STD_FP16_H__

#include "std/types.h"

typedef struct {
  int16_t integer;
  uint16_t fraction;
} Q16T;

Q16T CastFloatQ16(float value asm("fp0"));
Q16T CastIntQ16(int value asm("d0"));
void IAddQ16(Q16T *result asm("a0"), Q16T value asm("d0"));

#endif
