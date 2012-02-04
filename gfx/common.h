#ifndef __GFX2D_COMMON_H__
#define __GFX2D_COMMON_H__

#include <stdint.h>

#ifdef FALSE
#undef FALSE
#endif

#ifdef TRUE
#undef TRUE
#endif

typedef enum { FALSE, TRUE } bool;

#define abs(a) ((a) > 0 ? (a) : -(a))
#define swapi(a,b)	{ (a)^=(b); (b)^=(a); (a)^=(b); }
#define swapf(a,b)	{ float t; t = (b); (b) = (a); (a) = t; }

typedef struct Point {
  int16_t x, y;
} PointT;

typedef struct Rect {
  PointT tl, br;
} RectT;

#endif
