#include "std/math.h"
#include "distortions/generate.h"

void GenerateTwirlDistortion(DistortionMapT *map, float strenght) {
  float dy = 2.0f / (int)map->height;
  float dx = 2.0f / (int)map->width;
  float x, y;

  size_t i;

  float s = 2 * M_PI * strenght;

  for (y = -1.0f, i = 0; y < 1.0f; y += dy)
    for (x = -1.0f; x < 1.0f; x += dx, i++) {
      float r = sqrt(x * x + y * y);

      float u = x;
      float v = y;

      if (r <= 1) {
        float ang = atan2(x, y) + (1 - r) * s;

        u = sin(ang) * r;
        v = cos(ang) * r;
      }

      u = u / 2 + 0.5;
      v = v / 2 + 0.5;

      DistortionMapSet(map, i, u * map->textureH, v * map->textureW);
    }
}
