#ifndef __GFX_COMMON_H__
#define __GFX_COMMON_H__

#include "std/types.h"

typedef struct Point {
  int16_t x, y;
} PointT;

typedef struct Rect {
  PointT tl, br;
} RectT;

#endif
