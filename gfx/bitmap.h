#ifndef __GFX_BITMAP_H__
#define __GFX_BITMAP_H__

#include "gfx/common.h"

typedef struct bitmap {
  uint16_t width, height;
  uint8_t data[];
} bitmap_t;

bitmap_t *bitmap_new(int width, int height);
void bitmap_delete(bitmap_t *bitmap);

#endif
