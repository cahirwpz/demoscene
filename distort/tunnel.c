#include "std/math.h"
#include "distort/generate.h"

void GenerateTunnelDistortion(DistortionMapT *map,
                              int radius, float aspectRatio,
                              int centerX, int centerY)
{
  size_t x, y, i;

  for (y = 0, i = 0; y < map->height; y++) {
    float yc = ((int)y - centerY) * aspectRatio;
    
    for (x = 0; x < map->width; x++, i++) {
      float xc = (int)x - centerX;
      float d2 = xc * xc + yc * yc;

      float u = atan2(xc, yc) / (2 * M_PI);
      float v = (d2) ? ((float)radius / sqrt(d2)) : 0;

      DistortionMapSet(map, i, u, v);
    }
  }
}
