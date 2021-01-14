#include <blitter.h>

void BitmapSetArea(const BitmapT *bitmap, const Area2D *area, u_short color) {
  short i;

  BlitterSetAreaSetup(bitmap, area);
  for (i = 0; i < bitmap->depth; i++) {
    BlitterSetAreaStart(i, (color & (1 << i)) ? -1 : 0);
  }
}
