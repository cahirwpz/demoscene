#include "distort/generate.h"

void GenerateMapDistortion(DistortionMapT *map,
                           PixBufT *mapU, PixBufT *mapV,
                           float forceU, float forceV)
{
  float du = 1.0f / (int)map->height;
  float dv = 1.0f / (int)map->width;
  float u, v;
  size_t i;

  for (v = 0.0f, i = 0; v < 1.0f; v += dv) {
    for (u = 0.0f; u < 1.0f; u += du, i++) {
      float cu = (float)((int)mapU->data[i] - 128) / 128;
      float cv = (float)((int)mapV->data[i] - 128) / 128;

      DistortionMapSet(map, i,
                       (u + cu) * (int)map->textureW,
                       (v + cv) * (int)map->textureH);
    }
  }
}
