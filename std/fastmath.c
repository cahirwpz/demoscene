#include "std/fastmath.h"

/*
 * “Efficient approximations for the arctangent function”
 * Rajan, S. Sichun Wang Inkol, R. Joyal, A., May 2006
 */
float FastAtan(float x asm("fp0")) {
  return ((float)M_PI_4 - (fabsf(x) - 1.0f) * (0.2447f + 0.0663f * fabsf(x))) * x;
}

float FastAtan2(float dy asm("fp0"), float dx asm("fp1")) {
  float a;

  if (fabsf(dx) > fabsf(dy)) {
   a = FastAtan(dy / dx);
  } else {
    a = FastAtan(dx / dy);

    if (a < 0)
      a = -M_PI_2 - a;
    else
      a = M_PI_2 - a;
  }

  if (dx < 0) {
    if (dy < 0) {
      a = a - M_PI;
    } else {
      a = a + M_PI;
    }
  }

  return a;
}

/*
 * From Quake III Arena sources.
 */
float FastInvSqrt(float x asm("fp0")) {
  uint32_t i;
  float x2;

  x2 = x * 0.5f;
  i  = * (uint32_t *) &x;
  i  = 0x5f3759df - (i >> 1);
  x  = * (float *) &i;

  return x * (1.5f - (x2 * x * x));
}
