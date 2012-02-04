#ifndef __GFX_CANVAS_H__
#define __GFX_CANVAS_H__

#include "gfx/common.h"
#include "gfx/bitmap.h"

#define CANVAS_BITMAP_MANAGED 0x8000

typedef struct Canvas {
  BitmapT *bitmap;
  uint16_t flags;
  uint8_t fg_col;
  uint8_t bg_col;
  PointT pen_pos;
  RectangleT clip_area;
} CanvasT;

#define CanvasPenMoveTo(CANVAS, X, Y) { (CANVAS)->pen_pos.x = (X); (CANVAS)->pen_pos.y = (Y); }

CanvasT *NewCanvas(int width, int height);
void DeleteCanvas(CanvasT *canvas);

#endif
