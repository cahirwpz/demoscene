#include <blitter.h>

BitmapT *BitmapMakeMask(const BitmapT *bitmap) {
  BitmapT *mask = NewBitmap(bitmap->width, bitmap->height, 1, 0);
  u_short bltsize = (bitmap->height << 6) | (bitmap->bytesPerRow >> 1);
  void *const *planes = bitmap->planes;
  void *dst = mask->planes[0];
  short n = bitmap->depth;

  while (--n >= 0) {
    void *src = *planes++;

    WaitBlitter();

    custom->bltamod = 0;
    custom->bltbmod = 0;
    custom->bltdmod = 0;
    custom->bltcon0 = (SRCA | SRCB | DEST) | A_OR_B;
    custom->bltcon1 = 0;
    custom->bltafwm = -1;
    custom->bltalwm = -1;

    custom->bltapt = src;
    custom->bltbpt = dst;
    custom->bltdpt = dst;
    custom->bltsize = bltsize;
  }

  return mask;
}
