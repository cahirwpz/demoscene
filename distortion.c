#include <math.h>

#include "system/memory.h"

#include "distortion.h"

#define M_PI 3.14159265f

struct DistortionMap *NewDistortionMap(uint16_t width, uint16_t height) {
  struct DistortionMap *map = NEW_S(struct DistortionMap);

  if (map) {
    map->Width = width;
    map->Height = height;

    if (!(map->Map = NEW_AZ(uint16_t, width * height))) {
      DeleteDistortionMap(map);
      map = NULL;
    }
  }

  return map;
}

void DeleteDistortionMap(struct DistortionMap *map) {
  DELETE(map->Map);
  DELETE(map);
}

void GenerateTunnel(struct DistortionMap *tunnel,
                    int16_t radius, int16_t centerX, int16_t centerY) {
  int minX = -centerX;
  int minY = -centerY;
  int maxX = minX + tunnel->Width;
  int maxY = minY + tunnel->Height;
  uint16_t *map = tunnel->Map;

  int xi, yi;

  for (yi = minY; yi < maxY; yi++) {
    float y = (float)yi;

    for (xi = minX; xi < maxX; xi++) {
      float x = (float)xi;

      int offsetY = atan((float)x / (float)y) * 128.0f / M_PI;
      int offsetX = radius / sqrt(x * x + y * y);

      if (yi >= 0)
        offsetY += 128;

      *map++ = ((offsetY & 0xff) << 8) | (offsetX & 0xff);
    }
  }
}
