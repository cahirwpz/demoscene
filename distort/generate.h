#ifndef __DISTORT_GENERATE_H__
#define __DISTORT_GENERATE_H__

#include "distort/common.h"

typedef struct {
  size_t petals;
  float petalStart;
  float force;
} TunnelPetalsT;

void GenerateTunnelDistortion(DistortionMapT *tunnel,
                              float radius, float aspectRatio,
                              float centerX, float centerY,
                              TunnelPetalsT *petals);

void GenerateSineDistortion(DistortionMapT *map,
                            size_t xFreq, float xAmp, float xShift,
                            size_t yFreq, float yAmp, float yShift);
void GenerateTwirlDistortion(DistortionMapT *map,
                             float strenght, bool seamless);
void GenerateOffsetDistortion(DistortionMapT *map,
                              float uOffset, float vOffset);

#endif
