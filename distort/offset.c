#include "distort/generate.h"

void GenerateOffsetDistortion(DistortionMapT *map,
                              float uOffset, float vOffset)
{
  float du = 1.0f / (int)map->width;
  float dv = 1.0f / (int)map->height;
  float u, v;
  size_t i;

  for (v = 0.0f, i = 0; v < 1.0f; v += dv)
    for (u = 0.0f; u < 1.0f; u += du, i++)
      DistortionMapSet(map, i, u, v);
}
