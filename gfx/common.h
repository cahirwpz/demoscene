#ifndef __GFX_COMMON_H__
#define __GFX_COMMON_H__

#include "std/types.h"

typedef struct Point {
  size_t x, y;
} PointT;

typedef struct FPoint {
  float x, y;
} FPointT;

typedef struct Rect {
  size_t x, y;
  size_t w, h;
} RectT;

#endif
