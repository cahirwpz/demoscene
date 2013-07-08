#ifndef __DISTORT_COMMON_H__
#define __DISTORT_COMMON_H__

#include "std/types.h"
#include "std/fp16.h"
#include "gfx/pixbuf.h"

typedef struct {
  Q16T u, v;
} UV16T;

typedef enum {DMAP_OPTIMIZED, DMAP_ACCURATE} DistortionMapTypeT;

typedef struct DistortionMap {
  DistortionMapTypeT type;
	size_t width, height;
  size_t textureW, textureH;
  PtrT map;
} DistortionMapT;

DistortionMapT *NewDistortionMap(size_t width, size_t height,
                                 DistortionMapTypeT type,
                                 size_t textureW, size_t textureH);

DistortionMapT *NewDistortionMapFromFile(const StrT fileName);
void DistortionMapWriteToFile(DistortionMapT *map, const StrT fileName);

void DistortionMapSet(DistortionMapT *map asm("a0"), size_t i asm("d0"),
                      float u asm("fp0"), float v asm("fp1"));

void RenderDistortion(DistortionMapT *map, PixBufT *canvas, PixBufT *texture,
                      int offsetU, int offsetV);

#endif
