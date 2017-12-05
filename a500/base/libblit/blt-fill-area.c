#include "blitter.h"

__regargs void BlitterFillArea(BitmapT *bitmap, WORD plane, Area2D *area) {
  APTR bltpt = bitmap->planes[plane];
  UWORD bltmod, bltsize;

  if (area) {
    WORD x = area->x;
    WORD y = area->y; 
    WORD w = area->w;
    WORD h = area->h;

    bltpt += (((x + w) >> 3) & ~1) + ((y + h) * bitmap->bytesPerRow);
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
