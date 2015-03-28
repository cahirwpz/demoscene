#include "bltop.h"

__regargs BitmapT *BitmapMakeMask(BitmapT *bitmap) {
  BitmapT *mask = NewBitmap(bitmap->width, bitmap->height, 1);
  UWORD bltsize = (bitmap->height << 6) | (bitmap->bytesPerRow >> 1);
  APTR *planes = bitmap->planes;
  APTR dst = mask->planes[0];
  WORD n = bitmap->depth;

  while (--n >= 0) {
    APTR src = *planes++;

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
