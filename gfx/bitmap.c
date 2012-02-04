#include "gfx/bitmap.h"
#include "system/memory.h"

BitmapT *NewBitmap(int width, int height) {
  BitmapT *bitmap = NEW_Z(BitmapT, (width * height + sizeof(BitmapT)));

  if (bitmap) {
    bitmap->width = width;
    bitmap->height = height;
  }

  return bitmap;
}

void DeleteBitmap(BitmapT *bitmap) {
  DELETE(bitmap);
}
