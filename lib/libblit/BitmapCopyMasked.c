#include <blitter.h>

void BitmapCopyMasked(const BitmapT *dst, u_short x, u_short y,
                      const BitmapT *src, const BitmapT *msk) 
{
  short i, n = min(dst->depth, src->depth);

  BlitterCopyMaskedSetup(dst, x, y, src, msk);
  for (i = 0; i < n; i++)
    BlitterCopyMaskedStart(i, i);
}
