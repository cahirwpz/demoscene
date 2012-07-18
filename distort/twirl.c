#include "std/math.h"
#include "distort/generate.h"

void GenerateTwirlDistortion(DistortionMapT *map, float strenght) {
  float dy = 2.0f / (int)map->height;
  float dx = 2.0f / (int)map->width;
  float x, y;

  size_t i;

  float s = 2 * M_PI * strenght;

  for (y = -1.0f, i = 0; y < 1.0f; y += dy) {
    for (x = -1.0f; x < 1.0f; x += dx, i++) {
      float r = sqrt(x * x + y * y);

      float u = x;
      float v = y;

      if (r <= 1) {
        float ang = atan2(x, y) + (1 - r) * s;

        u = sin(ang) * r;
        v = cos(ang) * r;
      }

      DistortionMapSet(map, i, (u + 1.0f) * 0.5f, (v + 1.0f) * 0.5f);
    }
  }
}
