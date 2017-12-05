#include "blitter.h"

typedef struct {
  BitmapT *src;
  BitmapT *dst;
  ULONG src_start;
  ULONG dst_start;
  UWORD size;
  BOOL fast;
} StateT;

static StateT state[1];

void BlitterCopyAreaSetup(BitmapT *dst, UWORD x, UWORD y,
                          BitmapT *src, Area2D *area)
{
  UWORD dxo = x & 15;
  UWORD sxo = area->x & 15;
  UWORD wo = (dxo + area->w) & 15;
  BOOL reverse = dxo < sxo;
  UWORD width = (reverse ? sxo : dxo) + area->w;
  UWORD bytesPerRow = ((width + 15) & ~15) >> 3;
  UWORD srcmod = src->bytesPerRow - bytesPerRow;
  UWORD dstmod = dst->bytesPerRow - bytesPerRow;
  UWORD bltafwm = FirstWordMask[dxo];
  UWORD bltalwm = LastWordMask[wo];
  UWORD bltshift = rorw(reverse ? sxo - dxo : dxo - sxo, 4);

  /*
   * TODO: Two cases exist where number of word for 'src' and 'dst' differ.
   * 1. (sxo < (sxo + area->w) & 15) && (dxo > (dxo + area->w) & 15)
   * 2. (sxo > (sxo + area->w) & 15) && (dxo < (dxo + area->w) & 15)
   * To handle them an in-memory mask must be created as suggested by HRM:
   * http://amigadev.elowar.com/read/ADCD_2.1/Hardware_Manual_guide/node0121.html
   */

  state->src = src;
  state->dst = dst;
  state->src_start = ((area->x & ~15) >> 3) + area->y * src->bytesPerRow;
  state->dst_start = ((x & ~15) >> 3) + y * dst->bytesPerRow;
  state->size = (area->h << 6) | (bytesPerRow >> 1);

  if (reverse) {
    state->src_start += (area->h - 1) * src->bytesPerRow + bytesPerRow - 2;
    state->dst_start += (area->h - 1) * dst->bytesPerRow + bytesPerRow - 2;
  }

  state->fast = (dxo || wo) == 0;

  WaitBlitter();

  if (!state->fast) {
    custom->bltcon0 = (SRCB | SRCC | DEST) | (ABC | NABC | ABNC | NANBC);
    custom->bltcon1 = bltshift | (reverse ? BLITREVERSE : 0);
    custom->bltadat = -1;
    if (reverse) {
      custom->bltafwm = bltalwm;
      custom->bltalwm = bltafwm;
    } else {
      custom->bltafwm = bltafwm;
      custom->bltalwm = bltalwm;
    }
    custom->bltbmod = srcmod;
    custom->bltcmod = dstmod;
    custom->bltdmod = dstmod;
  } else {
    custom->bltcon0 = (SRCA | DEST) | A_TO_D;
    custom->bltcon1 = 0;
    custom->bltafwm = -1;
    custom->bltalwm = -1;
    custom->bltamod = srcmod;
    custom->bltdmod = dstmod;
  }
}

__regargs void BlitterCopyAreaStart(WORD dstbpl, WORD srcbpl) {
  APTR srcbpt = state->src->planes[srcbpl] + state->src_start;
  APTR dstbpt = state->dst->planes[dstbpl] + state->dst_start;
  UWORD bltsize = state->size;

  if (state->fast) {
    WaitBlitter();

    custom->bltapt = srcbpt;
    custom->bltdpt = dstbpt;
    custom->bltsize = bltsize;
  } else {
    WaitBlitter();

    custom->bltbpt = srcbpt;
    custom->bltcpt = dstbpt;
    custom->bltdpt = dstbpt;
    custom->bltsize = bltsize;
  }
}

void BitmapCopyArea(BitmapT *dst, UWORD x, UWORD y,
                    BitmapT *src, Area2D *area)
{
  WORD i, n = min(dst->depth, src->depth);

  BlitterCopyAreaSetup(dst, x, y, src, area);
  for (i = 0; i < n; i++)
    BlitterCopyAreaStart(i, i);
}
