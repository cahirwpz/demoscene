#ifndef __DISTORTIONS_GENERATE_H__
#define __DISTORTIONS_GENERATE_H__

#include "distortions/map.h"

void GenerateTunnel(DistortionMapT *tunnel,
                    int16_t radius, int16_t centerX, int16_t centerY);
void GenerateSineDistortion(DistortionMapT *map,
                            size_t xFreq, float xAmp, float xShift,
                            size_t yFreq, float yAmp, float yShift);
void GenerateTwirlDistortion(DistortionMapT *map, float strenght);

#endif
