#include "distort/generate.h"

void GenerateOffsetDistortion(DistortionMapT *map,
                              float uOffset, float vOffset)
{
  float du = (float)map->textureW / (int)map->width;
  float dv = (float)map->textureH / (int)map->height;
  float u, v;
  size_t i;

  for (v = 0, i = 0; v < (int)map->textureH; v += dv)
    for (u = 0; u < (int)map->textureW; u += du, i++)
      DistortionMapSet(map, i, u, v);
}
