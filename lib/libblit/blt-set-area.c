#include "blitter.h"

typedef struct {
  const BitmapT *bitmap;
  u_int start;
  u_short size;
} StateT;

static StateT state[1];

/* Supports any area dimensions,
 * but is optimized for 'x' and 'w' divisible by 16. */
void BlitterSetAreaSetup(const BitmapT *bitmap, const Area2D *area) {
  u_short bltafwm, bltalwm, bltmod, bytesPerRow;
  u_short x = 0, y = 0, width = bitmap->width, height = bitmap->height;

  if (area)
    x = area->x, y = area->y, width = area->w, height = area->h;

  width += x & 15;
  bytesPerRow = ((width + 15) & ~15) >> 3;

  bltafwm = FirstWordMask[x & 15];
  bltalwm = LastWordMask[width & 15];
  bltmod = bitmap->bytesPerRow - bytesPerRow;

  state->bitmap = bitmap;
  state->start = ((x & ~15) >> 3) + y * bitmap->bytesPerRow;
  state->size = (height << 6) | (bytesPerRow >> 1);

  WaitBlitter();

  if ((x & 15) || (width & 15)) {
    custom->bltadat = -1;
    custom->bltbmod = bltmod;
    custom->bltcon0 = (SRCB | DEST) | (NABC | NABNC | ABC | ANBC);
  } else {
    custom->bltcon0 = DEST | C_TO_D;
  }

  custom->bltcon1 = 0;
  custom->bltdmod = bltmod;
  custom->bltafwm = bltafwm;
  custom->bltalwm = bltalwm;
}

void BlitterSetAreaStart(short bplnum, u_short pattern) {
  void *bltpt = state->bitmap->planes[bplnum] + state->start;
  u_short bltsize = state->size;

  WaitBlitter();

  custom->bltcdat = pattern;
  custom->bltbpt = bltpt;
  custom->bltdpt = bltpt;
  custom->bltsize = bltsize;
}

void BitmapSetArea(const BitmapT *bitmap, const Area2D *area, u_short color) {
  short i;

  BlitterSetAreaSetup(bitmap, area);
  for (i = 0; i < bitmap->depth; i++) {
    BlitterSetAreaStart(i, (color & (1 << i)) ? -1 : 0);
  }
}
