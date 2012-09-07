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

#define GenerateMiscDistortion(NUM, U, V)                   \
void GenerateMisc ## NUM ## Distortion(DistortionMapT *map) \
{                                                           \
  float dx = 2.0f / (int)map->width;                        \
  float dy = 2.0f / (int)map->height;                       \
  int i, j, k;                                              \
                                                            \
  for (i = 0, k = 0; i < map->height; i++)                  \
    for (j = 0; j < map->width; j++, k++) {                 \
      float x = (float)(j - (int)map->width / 2) * dx;      \
      float y = (float)(i - (int)map->height / 2) * dy;     \
      float a = atan2(x, y);                                \
      float r = sqrt(x * x + y * y);                        \
      float u = (U);                                        \
      float v = (V);                                        \
      DistortionMapSet(map, k, u, v);                       \
    }                                                       \
}

#endif
