#ifndef __DISTORTIONS_MAP_H__
#define __DISTORTIONS_MAP_H__

#include "std/types.h"
#include "std/fp16.h"
#include "gfx/pixbuf.h"
#include "gfx/canvas.h"

typedef struct {
  Q16T u, v;
} TextureUV_Q16T;

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

void DistortionMapSet(DistortionMapT *map, size_t i, Q16T u, Q16T v);

void RenderDistortion(DistortionMapT *map, CanvasT *canvas, PixBufT *texture,
                      size_t offsetU, size_t offsetV);

#endif
