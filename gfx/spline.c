#include "std/memory.h"
#include "gfx/spline.h"

SplineT *NewSpline(size_t knots) {
  size_t size = sizeof(SplineT) + sizeof(SplineKnotT) * knots;
  SplineT *spline = (SplineT *)MemNew0(size, NULL);

  spline->knots = knots;

  return spline;
}

/*
 * http://en.wikipedia.org/wiki/Hermite_curve
 *
 * @param t: [0.0, 1.0]
 */
typedef struct CubicPolynomial {
  float h00;
  float h10;
  float h01;
  float h11;
} CubicPolynomialT;

static void HermiteCubicPolynomial(float t asm("fp0"),
                                   CubicPolynomialT *poly asm("a0"))
{
  /*
   * h00(t) = 2*t^3 - 3*t^2 + 1
   *        = 2*(t^3 - t^2) - (t^2 + 1)
   *        = 2*h11(t) - t^2 + 1
   *
   * h10(t) = t^3 - 2*t^2 + t
   *        = (t^3 - t^2) - t^2 + t
   *        = h11(t) - t^2 + t
   *
   * h01(t) = -2*t^3 + 3*t^2
   *        = -2*t^3 + 2*t^2 + t^2
   *        = -2*(t^3 - t^2) + t^2
   *        = -2*h11(t) + t^2
   *        = -(2*h11(t) - t^2)
   *
   * h11(t) = t^3 - t^2
   *
   * H(t) = h00(t) * p0 + h10(t) * m0 + h01(t) * p1 + h11(t) * m1
   */
  float t2 = t * t;
  float t3 = t2 * t;

  float h11 = t3 - t2;

  float dh11 = h11 + h11;
  float dh11mt2 = dh11 - t2;

  float h00 = dh11mt2 + 1;
  float h10 = h11 - t2 + t;
  float h01 = -dh11mt2;

  poly->h00 = h00;
  poly->h10 = h10;
  poly->h01 = h01;
  poly->h11 = h11;
}

/*
 * Spline evaluator implementation.
 */

struct SplineEval {
  SplineT *spline;

  /* internal state */
  size_t p; /* current point */
  float t;  /* Hermite's polynomial parameter */
};

static void DeleteSplineEval(SplineEvalT *eval) {
  MemUnref(eval->spline);
}

SplineEvalT *NewSplineEval(SplineT *spline) {
  SplineEvalT *iter = NewRecordGC(SplineEvalT, (FreeFuncT)DeleteSplineEval);

  iter->spline = spline;

  SplineEvalMoveTo(iter, 0);

  return iter;
}

bool SplineEvalMoveTo(SplineEvalT *eval, ssize_t point) {
  return FALSE;
}

bool SplineEvalAt(SplineEvalT *eval, float step, float *value) {
  return FALSE;
}

bool SplineEvalStepBy(SplineEvalT *eval, float step, float *value) {
  return FALSE;
}

/*
 * Spline iterator implementation.
 */

struct SplineIter {
  SplineT *spline;
  size_t steps;    /* steps per unit interval */

  /* cached values */
  float step;
  CubicPolynomialT *polyCache;

  /* internal state */
  size_t i; /* i-th step between two points */
  size_t p; /* current point */
  float t;  /* Hermite's polynomial parameter */
};

static void DeleteSplineIter(SplineIterT *iter) {
  MemUnref(iter->spline);
  MemUnref(iter->polyCache);
}

SplineIterT *NewSplineIter(SplineT *spline, size_t steps) {
  SplineIterT *iter = NewRecordGC(SplineIterT, (FreeFuncT)DeleteSplineIter);

  iter->spline = spline;
  iter->steps = steps;

  SplineIterReset(iter);

  if (steps > 1) {
    size_t i;

    iter->step = 1.0f / iter->steps;
    iter->polyCache = NewTable(CubicPolynomialT, iter->steps);

    for (i = 0; i < iter->steps; i++)
      HermiteCubicPolynomial(iter->step * i, &iter->polyCache[i]);
  }

  return iter;
}

void SplineIterReset(SplineIterT *iter) {
  iter->i = 0;
  iter->t = 0;
  iter->p = 0;
}

bool SplineIterNext(SplineIterT *iter, float *value) {
  if (iter->p < iter->spline->knots - 1) {
    CubicPolynomialT *poly = &iter->polyCache[iter->i];
    SplineKnotT *p0 = &iter->spline->knot[iter->p];
    SplineKnotT *p1 = &iter->spline->knot[iter->p + 1];

    iter->t += iter->step;
    iter->i++;

    if (iter->i == iter->steps) {
      iter->i = 0;
      iter->t = 0;
      iter->p++;
    }

    *value = poly->h00 * p0->value +
             poly->h10 * p0->tangent +
             poly->h01 * p1->value +
             poly->h11 * p1->tangent;

    return TRUE;
  }

  return FALSE;
}
