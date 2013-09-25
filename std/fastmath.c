#include "std/fastmath.h"

/*
 * “Efficient approximations for the arctangent function”
 * Rajan, S. Sichun Wang Inkol, R. Joyal, A., May 2006
 */
__regargs inline
float FastAtan(float x)  {
  return ((float)M_PI_4 - (fabsf(x) - 1.0f) * (0.2447f + 0.0663f * fabsf(x))) * x;
}

__regargs
float FastAtan2(float dy, float dx) {
  bool ge = fabsf(dx) > fabsf(dy);
  float a = ge ? (dy / dx) : (dx / dy);

  a = FastAtan(a);

  if (!ge)
    a = (a < 0) ? (- M_PI_2 - a) : (M_PI_2 - a);

  if (dx < 0)
    a = (dy < 0) ? (a - M_PI) : (a + M_PI);

  return a;
}

/*
 * From Quake III Arena sources.
 */
__regargs
float FastInvSqrt(float x) {
  uint32_t i;
  float x2;

  x2 = x * 0.5f;
  i  = * (uint32_t *) &x;
  i  = 0x5f3759df - (i >> 1);
  x  = * (float *) &i;

  return x * (1.5f - (x2 * x * x));
}
