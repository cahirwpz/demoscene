#ifndef __TOOLS_GRADIENT_H__
#define __TOOLS_GRADIENT_H__

#include "gfx/common.h"
#include "gfx/pixbuf.h"

typedef struct FLine {
  float x, y, dx, dy;
  float invNormalLength;
} FLineT;

void FLineInitFromPoints(FLineT *line, FPointT *p0, FPointT *p1);
void LinearGradient(PixBufT *map, const FLineT *line);
void CircularGradient(PixBufT *map, const FPointT *point);

#endif
