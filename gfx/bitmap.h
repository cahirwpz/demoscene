#ifndef __GFX_BITMAP_H__
#define __GFX_BITMAP_H__

#include "gfx/common.h"

typedef struct Bitmap {
  uint16_t width, height;
  uint8_t data[];
} BitmapT;

BitmapT *NewBitmap(int width, int height);
void DeleteBitmap(BitmapT *bitmap);

#endif
