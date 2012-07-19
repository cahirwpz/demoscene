#include "std/math.h"
#include "distort/generate.h"

void GenerateTunnelDistortion(DistortionMapT *map,
                              float radius, float aspectRatio,
                              float centerX, float centerY,
                              TunnelPetalsT *petals)
{
  float dx = 1.0f / (int)map->width;
  float dy = 1.0f / (int)map->height;
  float fx, fy;

  float maxX = max(fabs(centerX), fabs(1 + centerX)) * aspectRatio;
  float maxY = max(fabs(centerY), fabs(1 + centerY));
  float scaleD = 1.0f / sqrt(maxX * maxX + maxY * maxY);

  size_t x, y, i;

  radius /= sqrt((int)map->textureH * (int)map->textureH +
                 (int)map->textureW * (int)map->textureW);

  for (y = 0, fy = 0, i = 0; y < map->height; y++, fy += dy) {
    for (x = 0, fx = 0; x < map->width; x++, fx += dx, i++) {
      float yc = (fy - centerY) * aspectRatio;
      float xc = (fx - centerX);

      float a = atan2(xc, yc);
      float d = sqrt(xc * xc + yc * yc) * scaleD;
      float z;

      if (petals) {
        float petal = sin((int)petals->petals * a +
                          petals->petalStart * 2 * M_PI);

        d *= 1.0f + petal * petals->force;
      }

      z = -log2f(1.0f - d);

      DistortionMapSet(map, i, a / (2 * M_PI), radius / z);
    }
  }
}
