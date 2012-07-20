#ifndef __GFX_PROCEDURAL_H__
#define __GFX_PROCEDURAL_H__

#include "std/types.h"
#include "gfx/pixbuf.h"

typedef float (*GenPixelFuncT)(float x asm("fp0"), float y asm("fp1"),
                               PtrT data asm("a0"));

float LightNormalFalloff(float x asm("fp0"), float y asm("fp1"),
                         float *radius asm("a0"));
float LightLinearFalloff(float x asm("fp0"), float y asm("fp1"),
                         float *radius asm("a0"));
float LightLogarithmicFalloff(float x asm("fp0"), float y asm("fp1"),
                              float *radius asm("a0"));
float LightGaussianFalloff(float x asm("fp0"), float y asm("fp1"),
                           float *radius asm("a0"));

void GeneratePixels(PixBufT *dst, GenPixelFuncT func, PtrT data);

#endif
