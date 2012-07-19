#ifndef __DISTORT_GENERATE_H__
#define __DISTORT_GENERATE_H__

#include "distort/common.h"

void GenerateTunnelDistortion(DistortionMapT *tunnel,
                              int radius, int centerX, int centerY);
void GenerateSineDistortion(DistortionMapT *map,
                            size_t xFreq, float xAmp, float xShift,
                            size_t yFreq, float yAmp, float yShift);
void GenerateTwirlDistortion(DistortionMapT *map,
                             float strenght, bool seamless);
void GenerateOffsetDistortion(DistortionMapT *map,
                              float uOffset, float vOffset);

#endif
