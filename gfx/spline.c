#include "gfx/spline.h"

/*
 * http://en.wikipedia.org/wiki/Hermite_curve
 *
 * @param t: [0.0, 1.0]
 * @param p0: start point
 * @param p1: end point
 * @param m0: start tangent
 * @param m1: end tangent
 */
float HermiteCubicPolynomial(float t asm("fp0"),
                             float p0 asm("fp1"), float p1 asm("fp2"),
                             float m0 asm("fp3"), float m1 asm("fp4"))
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

  return h00 * p0 + h10 * m0 + h01 * p1 + h11 * m1;
}
