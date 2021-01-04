#include <blitter.h>

void BitmapCopyArea(const BitmapT *dst, u_short x, u_short y,
                    const BitmapT *src, const Area2D *area)
{
  short i, n = min(dst->depth, src->depth);

  BlitterCopyAreaSetup(dst, x, y, src, area);
  for (i = 0; i < n; i++)
    BlitterCopyAreaStart(i, i);
}
