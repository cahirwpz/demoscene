#include <math.h>

#include "txtgen/procedural.h"

static float DistanceFromCenter(PixBufT *pixbuf, size_t x, size_t y, float radius) {
  float radiusX = (int)pixbuf->width / 2;
  float radiusY = (int)pixbuf->height / 2;
  float distX = (x - radiusX) / radiusX;
  float distY = (y - radiusY) / radiusY;

  return __builtin_sqrtf(distX * distX + distY * distY) * radius;
}

float Light1(PixBufT *pixbuf, size_t x, size_t y, LightDataT *data) {
  return 1.0f - DistanceFromCenter(pixbuf, x, y, data->radius);
}

float Light2(PixBufT *pixbuf, size_t x, size_t y, LightDataT *data) {
  float d = DistanceFromCenter(pixbuf, x, y, data->radius);
  
  if (d == 0.0f)
    return 1.0f;

  return log(d * 0.5f) / log(0.1f);
}

float Light3(PixBufT *pixbuf, size_t x, size_t y, LightDataT *data) {
  float d = DistanceFromCenter(pixbuf, x, y, data->radius);

  return exp(-d * d / 0.25f) - 4.75f;
}

void GeneratePixels(PixBufT *dst, GenPixelFuncT func, PtrT data) {
  size_t x, y;

  for (y = 0; y < dst->height; y++) {
    for (x = 0; x < dst->width; x++) {
      float value = func(dst, x, y, data);

      if (value < 0.0f)
        value = 0.0f;

      if (value > 1.0f)
        value = 1.0f;
      
      dst->data[x + y * dst->width] = value * (dst->colors - 1) + dst->baseColor;
    }
  }
}
