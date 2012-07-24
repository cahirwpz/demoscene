#ifndef __STD_FASTMATH_H__
#define __STD_FASTMATH_H__

#include "std/math.h"

float FastAtan(float x asm("fp0"));
float FastAtan2(float dx asm("fp0"), float dy asm("fp1"));
float FastInvSqrt(float x asm("fp0"));

#endif
