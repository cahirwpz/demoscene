#include "blitter.h"

typedef struct {
  const BitmapT *src;
  const BitmapT *dst;
  u_int start;
  u_short size;
} StateT;

static StateT state[1];

void BlitterFastCopySetup(const BitmapT *dst, u_short x, u_short y,
                          const BitmapT *src) 
{
  u_short dstmod = dst->bytesPerRow - src->bytesPerRow;
  u_short bltshift = rorw(x & 15, 4);
  u_short bltsize = (src->height << 6) | (src->bytesPerRow >> 1);

  if (bltshift)
    bltsize++, dstmod -= 2;

  state->src = src;
  state->dst = dst;
  state->start = ((x & ~15) >> 3) + y * dst->bytesPerRow;
  state->size = bltsize;

  WaitBlitter();

  if (bltshift) {
    custom->bltalwm = 0;
    custom->bltamod = -2;
  } else {
    custom->bltalwm = -1;
    custom->bltamod = 0;
  }

  custom->bltdmod = dstmod;
  custom->bltcon0 = (SRCA | DEST | A_TO_D) | bltshift;
  custom->bltcon1 = 0;
  custom->bltafwm = -1;
}

void BlitterFastCopyStart(short dstbpl, short srcbpl) {
  void *srcbpt = state->src->planes[srcbpl];
  void *dstbpt = state->dst->planes[dstbpl] + state->start;
  u_short bltsize = state->size;

  WaitBlitter();

  custom->bltapt = srcbpt;
  custom->bltdpt = dstbpt;
  custom->bltsize = bltsize;
}

void BitmapCopyFast(const BitmapT *dst, u_short x, u_short y,
                    const BitmapT *src) 
{
  short i, n = min(dst->depth, src->depth);

  BlitterFastCopySetup(dst, x, y, src);
  for (i = 0; i < n; i++)
    BlitterFastCopyStart(i, i);
}
