#ifndef __DISTORTION_H__
#define __DISTORTION_H__

#include <stdint.h>

struct DistortionMap {
	uint16_t Width, Height;
	uint16_t *Map;
};

struct DistortionMap *NewDistortionMap(uint16_t width, uint16_t height);
void DeleteDistortionMap(struct DistortionMap *map);
void GenerateTunnel(struct DistortionMap *tunnel,
										int16_t radius, int16_t centerX, int16_t centerY);

void RenderDistortion(__reg("a2") UBYTE *chunky,
                      __reg("a3") struct DistortionMap *map,
                      __reg("a5") UBYTE *texture,
                      __reg("d0") UBYTE offsetX,
                      __reg("d1") UBYTE offsetY);

#endif
