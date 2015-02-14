#include "std/math.h"
#include "std/fastmath.h"
#include "tools/gradient.h"

void FLineInitFromPoints(FLineT *line, FPointT *p0, FPointT *p1) {
  float dx = p1->x - p0->x;
  float dy = p1->y - p0->y;

  line->x = p0->x;
  line->y = p0->y;
  line->dx = dx;
  line->dy = dy;
  line->invNormalLength = FastInvSqrt(dx * dx + dy * dy);
}

static inline float PointToLineDistance(const FLineT *line, float px, float py) {
  return fabsf((px - line->x) * line->dy - (py - line->x) * line->dx) *
    line->invNormalLength;
}

void LinearGradient(PixBufT *map, const FLineT *line) {
  uint8_t *data = map->data;
  int x, y;

  for (y = 0; y < map->height; y++) {
    for (x = 0; x < map->width; x++) {
      int d = (uint8_t)lroundf(PointToLineDistance(line, (float)x, (float)y));

      if (d < 0)
        d = 0;
      if (d >= 255)
        d = 255;

      *data++ = d;
    }
  }
}

void CircularGradient(PixBufT *map, const FPointT *point) {
  uint8_t *data = map->data;
  int x, y;

  for (y = 0; y < map->height; y++) {
    for (x = 0; x < map->width; x++) {
      int xc = x - point->x;
      int yc = y - point->y;
      int d = (uint8_t)lroundf(sqrtf((float)(xc * xc + yc * yc)));

      if (d < 0)
        d = 0;
      if (d >= 255)
        d = 255;

      *data++ = d;
    }
  }
}
