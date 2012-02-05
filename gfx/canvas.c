#include "system/memory.h"
#include "gfx/canvas.h"

static CanvasT *NewCanvasInternal(PixBufT *pixbuf, bool managed) {
  CanvasT *canvas = NEW_SZ(CanvasT);

  if (canvas) {
    canvas->_pixbuf = pixbuf;
    canvas->flags = managed ? CANVAS_BITMAP_MANAGED : 0;
    canvas->fg_col = 255;
    canvas->clip_area.br.x = pixbuf->width;
    canvas->clip_area.br.y = pixbuf->height;
  }

  return canvas;
}

CanvasT *NewCanvas(int width, int height) {
  PixBufT *pixbuf = NewPixBuf(width, height);

  if (pixbuf)
    return NewCanvasInternal(pixbuf, TRUE);

  return NULL;
}

void DeleteCanvas(CanvasT *canvas) {
  if (canvas) {
    if (canvas->flags & CANVAS_BITMAP_MANAGED)
      DeletePixBuf(canvas->_pixbuf);

    DELETE(canvas);
  }
}
