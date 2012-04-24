#include <string.h>

#include "std/memory.h"
#include "gfx/canvas.h"

CanvasT *NewCanvas(int width, int height) {
  CanvasT *canvas = NewRecord(CanvasT);

  canvas->pixbuf = NewPixBuf(width, height);
  canvas->fg_col = 255;
  canvas->clip_area.br.x = canvas->pixbuf->width;
  canvas->clip_area.br.y = canvas->pixbuf->height;

  return canvas;
}

void DeleteCanvas(CanvasT *canvas) {
  if (canvas) {
    DeletePixBuf(canvas->pixbuf);
    MemUnref(canvas);
  }
}

void CanvasFill(CanvasT *canvas, uint8_t color) {
  memset(canvas->pixbuf->data, color,
         canvas->clip_area.br.x * canvas->clip_area.br.y);
}
