#ifndef __GFX_PROCEDURAL_H__
#define __GFX_PROCEDURAL_H__

#include "std/types.h"
#include "gfx/pixbuf.h"

typedef float (*GenPixelFuncT)(PixBufT *pixbuf, size_t x, size_t y, PtrT data);

typedef struct LightData {
  float radius;
} LightDataT;

float Light1(PixBufT *pixbuf, size_t x, size_t y, LightDataT *data);
float Light2(PixBufT *pixbuf, size_t x, size_t y, LightDataT *data);
float Light3(PixBufT *pixbuf, size_t x, size_t y, LightDataT *data);

void GeneratePixels(PixBufT *dst, GenPixelFuncT func, PtrT data);

#endif
