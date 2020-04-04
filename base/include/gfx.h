#ifndef __GFX_H__
#define __GFX_H__

#include "common.h"

typedef struct {
  short x, y;
} Point2D;

typedef struct {
  short x1, y1;
  short x2, y2;
} Line2D;

typedef struct {
  short minX, minY;
  short maxX, maxY;
} Box2D;

typedef struct {
  short w, h;
} Size2D;

typedef struct {
  short x, y;
  short w, h;
} Area2D;

#include "bitmap.h"
#include "palette.h"

#endif
