#ifndef __GFX_SPLINE_H__
#define __GFX_SPLINE_H__

#include "std/types.h"

typedef struct SplineKnot {
  float value;
  float tangent;
} SplineKnotT;

typedef struct Spline {
  size_t knots;
  SplineKnotT knot[0];
} SplineT;

SplineT *NewSpline(size_t knots);

typedef struct SplineEval SplineEvalT;

SplineEvalT *NewSplineEval(SplineT *spline);
bool SplineEvalMoveTo(SplineEvalT *eval, ssize_t point);
bool SplineEvalAt(SplineEvalT *eval, float step, float *value);
bool SplineEvalStepBy(SplineEvalT *eval, float step, float *value);

typedef struct SplineIter SplineIterT;

SplineIterT *NewSplineIter(SplineT *spline, size_t steps);
void SplineIterReset(SplineIterT *iter);
bool SplineIterNext(SplineIterT *iter, float *value);

#endif
