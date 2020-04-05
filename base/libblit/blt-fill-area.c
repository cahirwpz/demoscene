#include "blitter.h"

__regargs void BlitterFillArea(const BitmapT *bitmap, short plane,
                               const Area2D *area)
{
  void *bltpt = bitmap->planes[plane];
  u_short bltmod, bltsize;

  if (area) {
    short x = area->x;
    short y = area->y; 
    short w = area->w;
    short h = area->h;

    bltpt += (((x + w) >> 3) & ~1) + (short)(y + h) * (short)bitmap->bytesPerRow;
    w >>= 3;
    bltmod = bitmap->bytesPerRow - w;
    bltsize = (h << 6) | (w >> 1);
  } else {
    bltpt += bitmap->bplSize;
    bltmod = 0;
    bltsize = (bitmap->height << 6) | (bitmap->bytesPerRow >> 1);
  }

  bltpt -= 2;

  WaitBlitter();

  custom->bltapt = bltpt;
  custom->bltdpt = bltpt;
  custom->bltamod = bltmod;
  custom->bltdmod = bltmod;
  custom->bltcon0 = (SRCA | DEST) | A_TO_D;
  custom->bltcon1 = BLITREVERSE | FILL_OR;
  custom->bltafwm = -1;
  custom->bltalwm = -1;
  custom->bltsize = bltsize;
}
