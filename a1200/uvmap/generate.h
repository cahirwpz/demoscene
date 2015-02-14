#ifndef __UVMAP_GENERATE_H__
#define __UVMAP_GENERATE_H__

#include "uvmap/common.h"

typedef struct {
  size_t petals;
  float petalStart;
  float force;
} TunnelPetalsT;

void UVMapGenerateTunnel(UVMapT *tunnel,
                         float radius, int wrap, float aspectRatio,
                         float centerX, float centerY, TunnelPetalsT *petals);
void UVMapGenerateSine(UVMapT *map,
                       size_t xFreq, float xAmp, float xShift,
                       size_t yFreq, float yAmp, float yShift);
void UVMapGenerateTwirl(UVMapT *map, float strenght, bool seamless);
void UVMapGenerateOffset(UVMapT *map, float uOffset, float vOffset);

#define UVMapGenerate(NAME, U, V)                              \
void UVMapGenerate ## NAME ## (UVMapT *map)                    \
{                                                              \
  float dx = 2.0f / (int)map->width;                           \
  float dy = 2.0f / (int)map->height;                          \
  int i, j, k;                                                 \
                                                               \
  for (i = 0, k = 0; i < map->height; i++)                     \
    for (j = 0; j < map->width; j++, k++) {                    \
      UNUSED float x = (float)(j - (int)map->width / 2) * dx;  \
      UNUSED float y = (float)(i - (int)map->height / 2) * dy; \
      UNUSED float a = atan2(x, y);                            \
      UNUSED float r = sqrt(x * x + y * y);                    \
      float u = (U);                                           \
      float v = (V);                                           \
      UVMapSet(map, k, u, v);                                  \
    }                                                          \
}

#endif
