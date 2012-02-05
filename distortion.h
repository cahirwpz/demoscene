#ifndef __DISTORTION_H__
#define __DISTORTION_H__

#include "std/types.h"
#include "gfx/pixbuf.h"
#include "gfx/canvas.h"

typedef struct DistortionMap {
	uint16_t Width, Height;
	uint16_t *Map;
} DistortionMapT;

DistortionMapT *NewDistortionMap(uint16_t width, uint16_t height);
void DeleteDistortionMap(DistortionMapT *map);
void GenerateTunnel(DistortionMapT *tunnel, int16_t radius, int16_t centerX,
                    int16_t centerY);

void RenderDistortion(__reg("a2") CanvasT *chunky,
                      __reg("a3") DistortionMapT *map,
                      __reg("a5") PixBufT *texture,
                      __reg("d0") uint8_t offsetX,
                      __reg("d1") uint8_t offsetY);

#endif
