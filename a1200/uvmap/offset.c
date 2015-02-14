#include "uvmap/generate.h"

void UVMapGenerateOffset(UVMapT *map, float uOffset, float vOffset) {
  float du = 1.0f / (int)map->width;
  float dv = 1.0f / (int)map->height;
  float u, v;
  int i;

  for (u = 0.0f, i = 0; u < 1.0f; u += du)
    for (v = 0.0f; v < 1.0f; v += dv, i++)
      UVMapSet(map, i, u, v);
}
