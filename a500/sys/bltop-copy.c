#include "bltop.h"

typedef struct {
  BitmapT *src;
  BitmapT *dst;
  ULONG start;
  UWORD size;
} StateT;

static StateT state[1];

/* Supports any (x, y) and any source bitmap width. */
__regargs void BlitterCopySetup(BitmapT *dst, UWORD x, UWORD y, BitmapT *src)
{
  /* Calculate real blit width. It can be greater than src->bytesPerRow! */
  UWORD width = (x & 15) + src->width;
  UWORD bytesPerRow = ((width + 15) & ~15) >> 3;
  UWORD srcmod = src->bytesPerRow - bytesPerRow;
  UWORD dstmod = dst->bytesPerRow - bytesPerRow;
  UWORD bltafwm = FirstWordMask[x & 15];
  UWORD bltalwm = LastWordMask[width & 15];
  UWORD bltshift = rorw(x & 15, 4);

  state->src = src;
  state->dst = dst;
  state->start = ((x & ~15) >> 3) + y * dst->bytesPerRow;
  state->size = (src->height << 6) | (bytesPerRow >> 1);

  WaitBlitter();

  if (bltshift) {
    custom->bltcon0 = (SRCB | SRCC | DEST) | (ABC | NABC | ABNC | NANBC);
    custom->bltbmod = srcmod;
    custom->bltadat = -1;
    custom->bltcmod = dstmod;
  } else {
    custom->bltamod = 0;
    custom->bltcon0 = (SRCA | DEST) | A_TO_D;
  }

  custom->bltcon1 = bltshift;
  custom->bltafwm = bltafwm;
  custom->bltalwm = bltalwm;
  custom->bltdmod = dstmod;
}

__regargs void BlitterCopyStart(WORD dstbpl, WORD srcbpl) {
  APTR srcbpt = state->src->planes[srcbpl];
  APTR dstbpt = state->dst->planes[dstbpl] + state->start;
  UWORD bltsize = state->size;

  WaitBlitter();

  custom->bltapt = srcbpt;
  custom->bltbpt = srcbpt;
  custom->bltcpt = dstbpt;
  custom->bltdpt = dstbpt;
  custom->bltsize = bltsize;
}

__regargs void BitmapCopy(BitmapT *dst, UWORD x, UWORD y, BitmapT *src) {
  WORD i, n = min(dst->depth, src->depth);

  BlitterCopySetup(dst, x, y, src);
  for (i = 0; i < n; i++)
    BlitterCopyStart(i, i);
}
