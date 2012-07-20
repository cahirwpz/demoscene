#include <string.h>

#include "std/memory.h"
#include "gfx/canvas.h"

static void DeleteCanvas(CanvasT *canvas) {
  MemUnref(canvas->pixbuf);
}

CanvasT *NewCanvas(int width, int height) {
  CanvasT *canvas = NewRecordGC(CanvasT, (FreeFuncT)DeleteCanvas);

  canvas->pixbuf = NewPixBuf(PIXBUF_CLUT, width, height);
  canvas->fg_col = 255;
  canvas->clip_area.br.x = canvas->pixbuf->width;
  canvas->clip_area.br.y = canvas->pixbuf->height;

  return canvas;
}

void CanvasFill(CanvasT *canvas, uint8_t color) {
  memset(canvas->pixbuf->data, color,
         canvas->clip_area.br.x * canvas->clip_area.br.y);
}
