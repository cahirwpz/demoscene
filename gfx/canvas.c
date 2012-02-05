#include "system/memory.h"
#include "gfx/canvas.h"

CanvasT *NewCanvas(int width, int height) {
  CanvasT *canvas = NEW_SZ(CanvasT);

  if (canvas) {
    canvas->pixbuf = NewPixBuf(width, height);
    canvas->fg_col = 255;
    canvas->clip_area.br.x = canvas->pixbuf->width;
    canvas->clip_area.br.y = canvas->pixbuf->height;

    if (canvas->pixbuf)
      return canvas;

    DeleteCanvas(canvas); 
  }

  return NULL;
}

void DeleteCanvas(CanvasT *canvas) {
  if (canvas) {
    DeletePixBuf(canvas->pixbuf);

    DELETE(canvas);
  }
}
