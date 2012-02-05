#ifndef __GFX_CANVAS_H__
#define __GFX_CANVAS_H__

#include "gfx/common.h"
#include "gfx/pixbuf.h"

#define CANVAS_BITMAP_MANAGED 0x8000

typedef struct Canvas {
  PixBufT *_pixbuf;
  uint16_t flags;
  uint8_t fg_col;
  uint8_t bg_col;
  PointT pen_pos;
  RectT clip_area;
} CanvasT;

#define CanvasPenMoveTo(CANVAS, X, Y) { (CANVAS)->pen_pos.x = (X); (CANVAS)->pen_pos.y = (Y); }

CanvasT *NewCanvas(int width, int height);
void DeleteCanvas(CanvasT *canvas);
inline size_t GetCanvasWidth(CanvasT *canvas) { return canvas->_pixbuf->width; }
inline uint8_t *GetCanvasPixelData(CanvasT *canvas) { return canvas->_pixbuf->data; }

#endif
