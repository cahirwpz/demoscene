#ifndef __GFX_CANVAS_H__
#define __GFX_CANVAS_H__

#include "gfx/common.h"
#include "gfx/bitmap.h"

#define CANVAS_BITMAP_MANAGED 0x8000

typedef struct canvas {
  bitmap_t *bitmap;
  uint16_t flags;
  uint8_t fg_col;
  uint8_t bg_col;
  point_t pen_pos;
  rectangle_t clip_area;
} canvas_t;

#define canvas_pen_move_to(CANVAS, X, Y) { (CANVAS)->pen_pos.x = (X); (CANVAS)->pen_pos.y = (Y); }

canvas_t *canvas_new(int width, int height);
void canvas_delete(canvas_t *canvas);

#endif
