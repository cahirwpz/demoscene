#ifndef __GFX2D_COMMON_H__
#define __GFX2D_COMMON_H__

#include "std/types.h"

typedef struct Point {
  int16_t x, y;
} PointT;

typedef struct Vertex {
  int16_t x, y, z;
} VertexT;

typedef struct Rect {
  PointT tl, br;
} RectT;

#endif
