#include "std/math.h"
#include "distort/generate.h"

void GenerateTunnelDistortion(DistortionMapT *map,
                              int radius, int centerX, int centerY)
{
  float scaleU = (int)map->textureW / (2 * M_PI);
  size_t x, y, i;

  for (y = 0, i = 0; y < map->height; y++) {
    int yi = (int)y - centerY;
    
    for (x = 0; x < map->width; x++, i++) {
      int xi = (int)x - centerX;
      int d2 = xi * xi + yi * yi;

      float u = 0.0f, v = 0.0f;

      if (yi != 0)
        u = atan((float)xi / yi) * scaleU;
      if (d2 != 0)
        v = (float)radius / sqrt(d2);

      DistortionMapSet(map, i, u, v);
    }
  }
}
