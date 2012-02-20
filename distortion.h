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

void RenderDistortion(DistortionMapT *map asm("a0"),
                      CanvasT *chunky asm("a1"),
                      PixBufT *texture asm("a2"),
                      uint8_t offsetX asm("d0"),
                      uint8_t offsetY asm("d1"));

#define RSC_DISTORTION_MAP(NAME, WIDTH, HEIGHT) \
  AddRscSimple(NAME, NewDistortionMap(WIDTH, HEIGHT), \
               (FreeFuncT)DeleteDistortionMap)


#endif
