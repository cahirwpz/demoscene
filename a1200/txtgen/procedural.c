#include "std/math.h"
#include "txtgen/procedural.h"

static float DistanceFromCenter(float x asm("fp0"), float y asm("fp1"), float radius asm("d0")) {
  x = (x - 0.5f) * 2.0f;
  y = (y - 0.5f) * 2.0f;

  return sqrtf(x * x + y * y) / radius;
}

float LightNormalFalloff(float x asm("fp0"), float y asm("fp1"), float *radius asm("a0")) {
  float d = DistanceFromCenter(x, y, *radius);

  return 1.0f / 16.0f * (1.0f / (d * d) - 1.0f);
}

float LightLinearFalloff(float x asm("fp0"), float y asm("fp1"), float *radius asm("a0")) {
  return 1.0f - DistanceFromCenter(x, y, *radius);
}

float LightLogarithmicFalloff(float x asm("fp0"), float y asm("fp1"), float *radius asm("a0")) {
  return -0.5f * logf(DistanceFromCenter(x, y, *radius));
}

float LightGaussianFalloff(float x asm("fp0"), float y asm("fp1"), float *radius asm("a0")) {
  float d = DistanceFromCenter(x, y, *radius);

  return (1.0f - exp(1.0f - d * d)) / (1 - M_E);
}

void GeneratePixels(PixBufT *dst, GenPixelFuncT func, PtrT data) {
  uint8_t *pixels = dst->data;

  float dx = 1.0f / dst->width;
  float dy = 1.0f / dst->height;
  float x, y;

  int colors = dst->lastColor - dst->baseColor;
  int baseColor = dst->baseColor;

  for (y = 0.0f; y < 1.0f; y += dy) {
    for (x = 0.0f; x < 1.0f; x += dx) {
      float value = func(x, y, data);

      if (value < 0.0f)
        value = 0.0f;

      if (value > 1.0f)
        value = 1.0f;
      
      *pixels++ = (int)(value * colors) + baseColor;
    }
  }
}
