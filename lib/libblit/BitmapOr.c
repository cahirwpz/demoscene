#include <blitter.h>

void BitmapOr(const BitmapT *dst, u_short x, u_short y, const BitmapT *src) {
  short i, n = min(dst->depth, src->depth);

  BlitterOrSetup(dst, x, y, src);
  for (i = 0; i < n; i++)
    BlitterOrStart(i, i);
}
