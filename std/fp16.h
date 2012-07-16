#ifndef __STD_FP16_H__
#define __STD_FP16_H__

#include "std/types.h"

typedef struct {
  int16_t integer;
  uint16_t fraction;
} Q16T;

void CastFloatQ16(Q16T *result asm("a0"), float value asm("fp0"));
void CastIntQ16(Q16T *result asm("a0"), int value asm("d0"));
void IAddQ16(Q16T *result asm("a0"), Q16T value asm("d0"));

#endif
