#include <blitter.h>

void BitmapCopy(const BitmapT *dst, u_short x, u_short y, const BitmapT *src) {
  short i, n = min(dst->depth, src->depth);

  BlitterCopySetup(dst, x, y, src);
  for (i = 0; i < n; i++)
    BlitterCopyStart(i, i);
}
