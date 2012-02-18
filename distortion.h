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

void RenderDistortion(CanvasT *chunky asm("a2"),
                      DistortionMapT *map asm("a3"),
                      PixBufT *texture asm("a5"),
                      uint8_t offsetX asm("d0"),
                      uint8_t offsetY asm("d1"));

#endif
