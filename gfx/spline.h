#ifndef __GFX_SPLINE_H__
#define __GFX_SPLINE_H__

#include "std/types.h"

typedef struct SplineKnot {
  float value;
  float tangent;
} SplineKnotT;

typedef struct Spline {
  size_t knots;
  bool closed;
  SplineKnotT knot[0];
} SplineT;

SplineT *NewSpline(size_t knots, bool closed);
float SplineEval(SplineT *spline asm("a0"), float t asm("fp0"));
void SplineInterpolate(SplineT *spline, size_t steps, PtrT array, SetItemFuncT writer);
void SplineAttachCatmullRomTangents(SplineT *spline);

#endif
