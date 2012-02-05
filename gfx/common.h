#ifndef __GFX_COMMON_H__
#define __GFX_COMMON_H__

#include "std/types.h"

typedef struct Color {
  uint8_t r,g,b;
  uint8_t pad;
} ColorT;

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
