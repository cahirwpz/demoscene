#ifndef __GFX_COMMON_H__
#define __GFX_COMMON_H__

#include "std/types.h"

typedef struct Point {
  int x, y;
} PointT;

typedef struct FPoint {
  float x, y;
} FPointT;

typedef struct Rect {
  int x, y;
  int w, h;
} RectT;

#endif
