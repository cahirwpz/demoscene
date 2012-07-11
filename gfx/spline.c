#include <math.h>
#include "std/memory.h"
#include "gfx/spline.h"

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
 * Spline implementation.
 */

SplineT *NewSpline(size_t knots, bool cyclic) {
  size_t size = sizeof(SplineT) + sizeof(SplineKnotT) * knots;
  SplineT *spline = (SplineT *)MemNew0(size, NULL);

  spline->knots = knots;

  return spline;
}

static float SplineEvalUnsafe(SplineT *spline, float t, size_t knot) {
  SplineKnotT *p0 = &spline->knot[knot];
  SplineKnotT *p1 = &spline->knot[knot + 1];
  CubicPolynomialT poly;

  HermiteCubicPolynomial(t, &poly);

  return poly.h00 * p0->value + poly.h10 * p0->tangent +
         poly.h01 * p1->value + poly.h11 * p1->tangent;
}

void SplineInterpolate(SplineT *spline, size_t steps, PtrT array, SetItemFuncT writer) {
  float stride = (float)spline->knots / steps;
  size_t step, knot;
  float t;

  for (step = 0, knot = 0, t = 0.0f; step < steps; step++) {
    float value = SplineEvalUnsafe(spline, t, knot);

    writer(array, step, &value);

    t += stride;

    while (t >= 1.0f) {
      t -= 1.0f;
      knot++;
    }
  }
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
  iter->p = 0;
  iter->t = 0.0f;

  return iter;
}

bool SplineEvalMoveTo(SplineEvalT *eval, ssize_t knot) {
  size_t knots = eval->spline->knots - 1;

  if ((knot < -knots) || (knot >= knots))
    return FALSE;

  if (knot < 0)
    knot += knots;

  eval->p = knot;
  eval->t = 0.0f;

  return TRUE;
}

bool SplineEvalAt(SplineEvalT *eval, float value, float *result) {
  size_t p = eval->p;
  float t = eval->t;

  bool success = SplineEvalStepBy(eval, value - (t + (float)p), result);

  eval->p = p;
  eval->t = t;

  return success;
}

bool SplineEvalStepBy(SplineEvalT *eval, float value, float *result) {
  float nt = eval->t + value;

  if ((nt >= 1.0f) || (nt < 0.0f)) {
    float nt_int = floor(nt);

    nt -= nt_int;

    if (!SplineEvalMoveTo(eval, eval->p + (int)nt_int))
      return FALSE;
  }

  eval->t = nt;

  *result = SplineEvalUnsafe(eval->spline, nt, eval->p);

  return TRUE;
}
