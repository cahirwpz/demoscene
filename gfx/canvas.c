#include "system/memory.h"
#include "gfx/canvas.h"

static canvas_t *canvas_new_internal(bitmap_t *bitmap, bool managed) {
  canvas_t *canvas = NEW_SZ(canvas_t);

  if (canvas) {
    canvas->bitmap = bitmap;
    canvas->flags = managed ? CANVAS_BITMAP_MANAGED : 0;
    canvas->fg_col = 255;
    canvas->clip_area.br.x = bitmap->width;
    canvas->clip_area.br.y = bitmap->height;
  }

  return canvas;
}

canvas_t *canvas_new(int width, int height) {
  bitmap_t *bitmap = bitmap_new(width, height);

  if (bitmap)
    return canvas_new_internal(bitmap, TRUE);

  return NULL;
}

void canvas_delete(canvas_t *canvas) {
  if (canvas->flags & CANVAS_BITMAP_MANAGED)
    bitmap_delete(canvas->bitmap);

  DELETE(canvas);
}
