#ifndef __GFX_SPLINE_H__
#define __GFX_SPLINE_H__

#include "std/types.h"

float HermiteCubicPolynomial(float t asm("fp0"),
                             float p0 asm("fp1"), float p1 asm("fp2"),
                             float m0 asm("fp3"), float m1 asm("fp4"));

#endif
