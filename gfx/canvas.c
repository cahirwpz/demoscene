#include "system/memory.h"
#include "gfx/canvas.h"

static CanvasT *NewCanvasInternal(BitmapT *bitmap, bool managed) {
  CanvasT *canvas = NEW_SZ(CanvasT);

  if (canvas) {
    canvas->bitmap = bitmap;
    canvas->flags = managed ? CANVAS_BITMAP_MANAGED : 0;
    canvas->fg_col = 255;
    canvas->clip_area.br.x = bitmap->width;
    canvas->clip_area.br.y = bitmap->height;
  }

  return canvas;
}

CanvasT *NewCanvas(int width, int height) {
  BitmapT *bitmap = NewBitmap(width, height);

  if (bitmap)
    return NewCanvasInternal(bitmap, TRUE);

  return NULL;
}

void DeleteCanvas(CanvasT *canvas) {
  if (canvas) {
    if (canvas->flags & CANVAS_BITMAP_MANAGED)
      DeleteBitmap(canvas->bitmap);

    DELETE(canvas);
  }
}
