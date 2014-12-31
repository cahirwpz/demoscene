#include "blitter.h"
#include "hardware.h"

__regargs void BlitterClear(BitmapT *bitmap, UWORD plane) {
  custom->bltadat = 0;
  custom->bltdpt = bitmap->planes[plane];
  custom->bltdmod = 0;
  custom->bltcon0 = DEST;
  custom->bltcon1 = 0;
  custom->bltsize = (bitmap->height << 6) | (bitmap->bytesPerRow >> 1);
}

__regargs void BlitterClearSync(BitmapT *bitmap, UWORD plane) {
  APTR bltdpt = bitmap->planes[plane];
  UWORD bltsize = (bitmap->height << 6) | (bitmap->bytesPerRow >> 1);

  WaitBlitter();
  custom->bltadat = 0;
  custom->bltdpt = bltdpt;
  custom->bltdmod = 0;
  custom->bltcon0 = DEST;
  custom->bltcon1 = 0;
  custom->bltsize = bltsize;
}

__regargs void BlitterFill(BitmapT *bitmap, UWORD plane) {
  UBYTE *bpl = bitmap->planes[plane] + bitmap->bplSize - 2;

  custom->bltapt = bpl;
  custom->bltdpt = bpl;
  custom->bltamod = 0;
  custom->bltdmod = 0;
  custom->bltcon0 = (SRCA | DEST) | A_TO_D;
  custom->bltcon1 = BLITREVERSE | FILL_OR;
  custom->bltafwm = -1;
  custom->bltalwm = -1;
  custom->bltsize = (bitmap->height << 6) | (bitmap->bytesPerRow >> 1);
}

__regargs void BlitterFillSync(BitmapT *bitmap, UWORD plane) {
  APTR bltpt = bitmap->planes[plane] + bitmap->bplSize - 2;
  UWORD bltsize = (bitmap->height << 6) | (bitmap->bytesPerRow >> 1);

  WaitBlitter();
  custom->bltapt = bltpt;
  custom->bltdpt = bltpt;
  custom->bltamod = 0;
  custom->bltdmod = 0;
  custom->bltcon0 = (SRCA | DEST) | A_TO_D;
  custom->bltcon1 = BLITREVERSE | FILL_OR;
  custom->bltafwm = -1;
  custom->bltalwm = -1;
  custom->bltsize = bltsize;
}

void BlitterCopySync(BitmapT *dst, UWORD dstbpl, UWORD x, UWORD y,
                     BitmapT *src, UWORD srcbpl) 
{
  APTR srcbpt = (APTR)src->planes[srcbpl];
  APTR dstbpt = (APTR)dst->planes[dstbpl] + ((x & ~15) >> 3) + y * dst->bytesPerRow;
  UWORD w = src->bytesPerRow;

  WaitBlitter();

  if (x & 15) {
    w += 2;

    custom->bltadat = 0xffff;
    custom->bltbpt = srcbpt;
    custom->bltcpt = dstbpt;
    custom->bltbmod = -2;
    custom->bltcmod = dst->bytesPerRow - w;
    custom->bltcon0 = (SRCB | SRCC | DEST) | (ABC | NABC | ABNC | NANBC) | ((x & 15) << ASHIFTSHIFT);
    custom->bltcon1 = ((x & 15) << BSHIFTSHIFT);
    custom->bltalwm = 0;
  } else {
    custom->bltapt = srcbpt;
    custom->bltamod = 0;
    custom->bltcon0 = (SRCA | DEST) | A_TO_D;
    custom->bltcon1 = 0;
    custom->bltalwm = -1;
  }

  custom->bltdpt = dstbpt;
  custom->bltdmod = dst->bytesPerRow - w;
  custom->bltafwm = -1;
  custom->bltsize = (src->height << 6) | (w >> 1);
}

void BlitterCopyMaskedSync(BitmapT *dst, UWORD dstbpl, UWORD x, UWORD y,
                           BitmapT *src, UWORD srcbpl, BitmapT *msk) 
{
  APTR srcbpt = (APTR)src->planes[srcbpl];
  APTR mskbpt = (APTR)msk->planes[0];
  APTR dstbpt = (APTR)dst->planes[dstbpl] + ((x & ~15) >> 3) + y * dst->bytesPerRow;
  UWORD w = src->bytesPerRow;

  WaitBlitter();

  if (x & 15) {
    w += 2;

    custom->bltamod = -2;
    custom->bltbmod = -2;
    custom->bltcon0 = (SRCA | SRCB | SRCC | DEST) | (ABC | ABNC | ANBC | NANBC) | ((x & 15) << ASHIFTSHIFT);
    custom->bltcon1 = ((x & 15) << BSHIFTSHIFT);
    custom->bltalwm = 0;
  } else {
    custom->bltamod = 0;
    custom->bltbmod = 0;
    custom->bltcon0 = (SRCA | SRCB | SRCC | DEST) | (ABC | ABNC | ANBC | NANBC);
    custom->bltcon1 = 0;
    custom->bltalwm = -1;
  }

  custom->bltapt = srcbpt;
  custom->bltbpt = mskbpt;
  custom->bltcpt = dstbpt;
  custom->bltdpt = dstbpt;
  custom->bltcmod = dst->bytesPerRow - w;
  custom->bltdmod = dst->bytesPerRow - w;
  custom->bltafwm = -1;
  custom->bltsize = (src->height << 6) | (w >> 1);
}

void BlitterCopyAreaSync(BitmapT *dst, UWORD dstbpl,
                         UWORD dx, UWORD dy,
                         BitmapT *src, UWORD srcbpl,
                         UWORD sx, UWORD sy, UWORD sw, UWORD sh)
{
  APTR srcbpt = (APTR)src->planes[srcbpl] + ((sx & ~15) >> 3) + sy * src->bytesPerRow;
  APTR dstbpt = (APTR)dst->planes[dstbpl] + ((dx & ~15) >> 3) + dy * dst->bytesPerRow;

  WaitBlitter();

  sw >>= 3;

  /* sx and sw must be multiply of 16 */

  if (0) { //(dx & 15) {
    sw += 2;

    custom->bltadat = 0xffff;
    custom->bltbpt = srcbpt;
    custom->bltcpt = dstbpt;
    custom->bltbmod = src->bytesPerRow - sw;
    custom->bltcmod = dst->bytesPerRow - sw;
    custom->bltcon0 = (SRCB | SRCC | DEST) | (ABC | NABC | ABNC | NANBC) | ((dx & 15) << ASHIFTSHIFT);
    custom->bltcon1 = ((dx & 15) << BSHIFTSHIFT);
    custom->bltalwm = 0;
  } else {
    custom->bltapt = srcbpt;
    custom->bltamod = src->bytesPerRow - sw;
    custom->bltcon0 = (SRCA | DEST) | A_TO_D;
    custom->bltcon1 = 0;
    custom->bltalwm = -1;
  }

  custom->bltdpt = dstbpt;
  custom->bltdmod = dst->bytesPerRow - sw;
  custom->bltafwm = -1;
  custom->bltsize = (sh << 6) | (sw >> 1);
}

void BlitterSetSync(BitmapT *dst, UWORD dstbpl, UWORD x, UWORD y, UWORD w, UWORD h, UWORD val) {
  APTR dstbpt = (APTR)dst->planes[dstbpl] + ((x & ~15) >> 3) + y * dst->bytesPerRow;

  w >>= 3;

  WaitBlitter();

  if (x & 15) {
    w += 2;

    custom->bltadat = 0xffff;
    custom->bltbpt = dstbpt;
    custom->bltcdat = val;
    custom->bltbmod = -2;
    custom->bltcmod = dst->bytesPerRow - w;
    custom->bltcon0 = (SRCB | DEST) | (NABC | NABNC | ABC | ANBC) | ((x & 15) << ASHIFTSHIFT);
    custom->bltcon1 = ((x & 15) << BSHIFTSHIFT);
  } else {
    custom->bltadat = val;
    custom->bltamod = 0;
    custom->bltcon0 = DEST | A_TO_D;
    custom->bltcon1 = 0;
  }

  custom->bltdpt = dstbpt;
  custom->bltdmod = dst->bytesPerRow - w;
  custom->bltalwm = -1;
  custom->bltafwm = -1;
  custom->bltsize = (h << 6) | (w >> 1);
}

void BlitterSetMaskedSync(BitmapT *dst, UWORD dstbpl, UWORD x, UWORD y,
                          BitmapT *msk, UWORD val)
{
  APTR mskbpt = (APTR)msk->planes[0];
  APTR dstbpt = (APTR)dst->planes[dstbpl] + ((x & ~15) >> 3) + y * dst->bytesPerRow;
  UWORD w = msk->bytesPerRow;

  WaitBlitter();

  if (x & 15) {
    w += 2;

    custom->bltbmod = -2;
    custom->bltcon0 = (SRCB | SRCC | DEST) | (ABC | ABNC | ANBC | NANBC) | ((x & 15) << ASHIFTSHIFT);
    custom->bltcon1 = ((x & 15) << BSHIFTSHIFT);
    custom->bltalwm = 0;
  } else {
    custom->bltbmod = 0;
    custom->bltcon0 = (SRCB | SRCC | DEST) | (ABC | ABNC | ANBC | NANBC);
    custom->bltcon1 = 0;
    custom->bltalwm = -1;
  }

  custom->bltadat = val;
  custom->bltbpt = mskbpt;
  custom->bltcpt = dstbpt;
  custom->bltdpt = dstbpt;
  custom->bltcmod = dst->bytesPerRow - w;
  custom->bltdmod = dst->bytesPerRow - w;
  custom->bltafwm = -1;
  custom->bltsize = (msk->height << 6) | (w >> 1);
}

/* Bitplane adder with saturation. */
void BlitterAddSaturatedSync(BitmapT *dst_bm, WORD dx, WORD dy, BitmapT *src_bm, BitmapT *carry_bm) {
  ULONG dst_begin = ((dx & ~15) >> 3) + dy * (WORD)dst_bm->bytesPerRow;
  UWORD dst_modulo = (dst_bm->bytesPerRow - src_bm->bytesPerRow) - 2;
  UWORD src_shift = (dx & 15) << ASHIFTSHIFT;
  UWORD bltsize = (src_bm->height << 6) | ((src_bm->width + 16) >> 4);
  APTR carry0 = carry_bm->planes[0];
  APTR carry1 = carry_bm->planes[1];
  APTR *src = src_bm->planes;
  APTR *dst = dst_bm->planes;

  {
    APTR aptr = (*src++);
    APTR bptr = (*dst++) + dst_begin;

    WaitBlitter();

    /* Initialize blitter */
    custom->bltamod = -2;
    custom->bltbmod = dst_modulo;
    custom->bltcmod = 0;
    custom->bltcon1 = 0;
    custom->bltafwm = -1;
    custom->bltalwm = 0;

    /* Bitplane 0: half adder with carry. */
    custom->bltapt = aptr;
    custom->bltbpt = bptr;
    custom->bltdpt = carry0;
    custom->bltdmod = 0;
    custom->bltcon0 = HALF_ADDER_CARRY | src_shift;
    custom->bltsize = bltsize;

    WaitBlitter();
    custom->bltapt = aptr;
    custom->bltbpt = bptr;
    custom->bltdpt = bptr;
    custom->bltdmod = dst_modulo;
    custom->bltcon0 = HALF_ADDER | src_shift;
    custom->bltsize = bltsize;
  }

  {
    WORD n = dst_bm->depth - 1;

    /* Bitplane 1-n: full adder with carry. */
    while (--n >= 0) {
      APTR aptr = (*src++);
      APTR bptr = (*dst++) + dst_begin;

      WaitBlitter();
      custom->bltapt = aptr;
      custom->bltbpt = bptr;
      custom->bltcpt = carry0;
      custom->bltdpt = carry1;
      custom->bltdmod = 0;
      custom->bltcon0 = FULL_ADDER_CARRY | src_shift;
      custom->bltsize = bltsize;

      WaitBlitter();
      custom->bltapt = aptr;
      custom->bltbpt = bptr;
      custom->bltcpt = carry0;
      custom->bltdpt = bptr;
      custom->bltdmod = dst_modulo;
      custom->bltcon0 = FULL_ADDER | src_shift;
      custom->bltsize = bltsize;

      swapr(carry0, carry1);
    }
  }

  /* Apply saturation bits. */
  {
    WORD n = dst_bm->depth;

    dst = dst_bm->planes;

    WaitBlitter();
    custom->bltamod = dst_modulo;
    custom->bltbmod = 0;
    custom->bltdmod = dst_modulo;
    custom->bltcon0 = (SRCA | SRCB | DEST) | A_OR_B;
    custom->bltalwm = -1;

    while (--n >= 0) {
      APTR bptr = (*dst++) + dst_begin;

      WaitBlitter();
      custom->bltapt = bptr;
      custom->bltbpt = carry0;
      custom->bltdpt = bptr;
      custom->bltsize = bltsize;
    }
  }
}

/*
 * Minterm is either:
 * - OR: (ABC | ABNC | NABC | NANBC)
 * - XOR: (ABNC | NABC | NANBC)
 */

/*
 *  \   |   /
 *   \3 | 1/
 *  7 \ | / 6
 *     \|/
 *  ----X----
 *     /|\
 *  5 / | \ 4
 *   /2 | 0\
 *  /   |   \
 *
 * OCT | SUD SUL AUL
 * ----+------------
 *   3 |   1   1   1
 *   0 |   1   1   0
 *   4 |   1   0   1
 *   7 |   1   0   0
 *   2 |   0   1   1
 *   5 |   0   1   0
 *   1 |   0   0   1
 *   6 |   0   0   0
 */

struct {
  UBYTE *data;
  UBYTE *scratch;
  WORD stride;
  UWORD bltcon0;
  UWORD bltcon1;
} line;

__regargs void BlitterLineSetup(BitmapT *bitmap, UWORD plane, UWORD bltcon0, UWORD bltcon1) 
{
  line.data = bitmap->planes[plane];
  line.scratch = bitmap->planes[bitmap->depth];
  line.stride = bitmap->bytesPerRow;
  line.bltcon0 = bltcon0;
  line.bltcon1 = bltcon1;

  custom->bltafwm = -1;
  custom->bltalwm = -1;
  custom->bltadat = 0x8000;
  custom->bltbdat = 0xffff; /* Line texture pattern. */
  custom->bltcmod = line.stride;
  custom->bltdmod = line.stride;
}

__regargs void BlitterLine(WORD x1, WORD y1, WORD x2, WORD y2) {
  UBYTE *data = line.data;
  UWORD bltcon1 = line.bltcon1;
  WORD dmax, dmin, derr;

  /* Always draw the line downwards. */
  if (y1 > y2) {
    swapr(x1, x2);
    swapr(y1, y2);
  }

  /* Word containing the first pixel of the line. */
  data += line.stride * y1;
  data += (x1 >> 3) & ~1;

  dmax = x2 - x1;
  dmin = y2 - y1;

  if (dmax < 0)
    dmax = -dmax;

  if (dmax >= dmin) {
    if (x1 >= x2)
      bltcon1 |= (AUL | SUD);
    else
      bltcon1 |= SUD;
  } else {
    swapr(dmax, dmin);
    if (x1 >= x2)
      bltcon1 |= SUL;
  }

  derr = 2 * dmin - dmax;
  if (derr < 0)
    bltcon1 |= SIGNFLAG;

  custom->bltcon0 = rorw(x1 & 15, 4) | line.bltcon0;
  custom->bltcon1 = rorw(x1 & 15, 4) | bltcon1;

  custom->bltamod = derr - dmax;
  custom->bltbmod = 2 * dmin;

  custom->bltapt = (APTR)(LONG)derr;
  custom->bltcpt = data;
  /* Uses undocumented chipset feature.
   * First dot is drawn into bltdpt, the rest goes to bltcpt. */
  custom->bltdpt = (bltcon1 & ONEDOT) ? line.scratch : data;

  custom->bltsize = (dmax << 6) + 66;
}

__regargs void BlitterLineSync(WORD x1, WORD y1, WORD x2, WORD y2) {
  UBYTE *data = line.data;
  UWORD bltcon1 = line.bltcon1;
  WORD dmax, dmin, derr;

  /* Always draw the line downwards. */
  if (y1 > y2) {
    swapr(x1, x2);
    swapr(y1, y2);
  }

  /* Word containing the first pixel of the line. */
  data += line.stride * y1;
  data += (x1 >> 3) & ~1;

  dmax = x2 - x1;
  dmin = y2 - y1;

  if (dmax < 0)
    dmax = -dmax;

  if (dmax >= dmin) {
    if (x1 >= x2)
      bltcon1 |= (AUL | SUD);
    else
      bltcon1 |= SUD;
  } else {
    swapr(dmax, dmin);
    if (x1 >= x2)
      bltcon1 |= SUL;
  }

  derr = 2 * dmin - dmax;
  if (derr < 0)
    bltcon1 |= SIGNFLAG;
  bltcon1 |= rorw(x1 & 15, 4);

  {
    UWORD bltcon0 = rorw(x1 & 15, 4) | line.bltcon0;
    UWORD bltamod = derr - dmax;
    UWORD bltbmod = 2 * dmin;
    APTR bltapt = (APTR)(LONG)derr;
    APTR bltdpt = (bltcon1 & ONEDOT) ? line.scratch : data;
    UWORD bltsize = (dmax << 6) + 66;

    WaitBlitter();

    custom->bltcon0 = bltcon0;
    custom->bltcon1 = bltcon1;
    custom->bltamod = bltamod;
    custom->bltbmod = bltbmod;
    custom->bltapt = bltapt;
    custom->bltcpt = data;
    custom->bltdpt = bltdpt;
    custom->bltsize = bltsize;
  }
}
