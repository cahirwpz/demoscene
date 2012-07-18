#include "std/math.h"
#include "distort/generate.h"

void GenerateTunnelDistortion(DistortionMapT *map,
                              int radius, int centerX, int centerY)
{
  size_t x, y, i;

  for (y = 0, i = 0; y < map->height; y++) {
    int yi = (int)y - centerY;
    
    for (x = 0; x < map->width; x++, i++) {
      int xi = (int)x - centerX;
      int d2 = xi * xi + yi * yi;

      float u = atan2(xi, yi) / (2 * M_PI);
      float v = (d2) ? ((float)radius / sqrt(d2)) : 0;

      DistortionMapSet(map, i, u, v);
    }
  }
}
