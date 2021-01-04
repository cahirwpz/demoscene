#include <blitter.h>

void BitmapCopyFast(const BitmapT *dst, u_short x, u_short y,
                    const BitmapT *src) 
{
  short i, n = min(dst->depth, src->depth);

  BlitterCopyFastSetup(dst, x, y, src);
  for (i = 0; i < n; i++)
    BlitterCopyFastStart(i, i);
}
