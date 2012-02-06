#ifndef __GFX_CANVAS_H__
#define __GFX_CANVAS_H__

#include "gfx/common.h"
#include "gfx/pixbuf.h"

typedef struct Canvas {
  PixBufT *pixbuf;
  uint8_t fg_col;
  uint8_t bg_col;
  PointT pen_pos;
  RectT clip_area;
} CanvasT;

CanvasT *NewCanvas(int width, int height);
void DeleteCanvas(CanvasT *canvas);

void CanvasFill(CanvasT *canvas, uint8_t color);

inline size_t GetCanvasWidth(CanvasT *canvas) { return canvas->pixbuf->width; }
inline uint8_t *GetCanvasPixelData(CanvasT *canvas) { return canvas->pixbuf->data; }
inline void CanvasPenMoveTo(CanvasT *canvas, int16_t x, int16_t y) { canvas->pen_pos.x = x; canvas->pen_pos.y = y; }

#endif
