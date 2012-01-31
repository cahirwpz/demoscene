#include "gfx/bitmap.h"
#include "system/memory.h"

bitmap_t *bitmap_new(int width, int height) {
  bitmap_t *bitmap = NEW_Z(bitmap_t, (width * height + sizeof(bitmap_t)));

  if (bitmap) {
    bitmap->width = width;
    bitmap->height = height;
  }

  return bitmap;
}

void bitmap_delete(bitmap_t *bitmap) {
  DELETE(bitmap);
}
