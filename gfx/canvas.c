#include <string.h>

#include "std/memory.h"
#include "gfx/canvas.h"

static void DeleteCanvas(CanvasT *canvas) {
  MemUnref(canvas->pixbuf);
}

TYPEDECL(CanvasT, (FreeFuncT)DeleteCanvas);

CanvasT *NewCanvas(int width, int height) {
  CanvasT *canvas = NewInstance(CanvasT);

  canvas->pixbuf = NewPixBuf(PIXBUF_CLUT, width, height);
  canvas->fg_col = 255;
  canvas->clip_area.w = canvas->pixbuf->width;
  canvas->clip_area.h = canvas->pixbuf->height;

  return canvas;
}

void CanvasFill(CanvasT *canvas, uint8_t color) {
  memset(canvas->pixbuf->data, color,
         canvas->clip_area.w * canvas->clip_area.h);
}
