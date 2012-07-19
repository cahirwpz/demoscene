#include "std/math.h"
#include "distort/generate.h"

void GenerateTwirlDistortion(DistortionMapT *map, float strenght, bool seamless) {
  float dy = 2.0f / (int)map->height;
  float dx = 2.0f / (int)map->width;
  float x, y;

  size_t xi, yi, i;

  float s = 2 * M_PI * strenght;

  for (y = -1.0f, i = 0, yi = 0; yi < map->height; yi++, y += dy) {
    for (x = -1.0f, xi = 0; xi < map->width; xi++, x += dx, i++) {
      float r = sqrt(x * x + y * y);

      float u = x;
      float v = y;

      if ((r <= 1) || (!seamless)) {
        float rad = atan2(x, y) + (1 - r) * (1 - r) * s;

        u = sin(rad) * r;
        v = cos(rad) * r;
      }

      DistortionMapSet(map, i, (u + 1.0f) * 0.5f, (v + 1.0f) * 0.5f);
    }
  }
}
