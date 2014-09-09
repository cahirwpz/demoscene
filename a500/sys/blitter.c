#include "blitter.h"
#include "hardware.h"

__regargs void BlitterClear(BitmapT *bitmap, UWORD plane) {
  custom->bltadat = 0;
  custom->bltdpt = bitmap->planes[plane];
  custom->bltdmod = 0;
  custom->bltcon0 = DEST;
  custom->bltcon1 = 0;
  custom->bltsize = (bitmap->height << 6) + (bitmap->width >> 4);
}

__regargs void BlitterFill(BitmapT *bitmap, UWORD plane) {
  UBYTE *bpl = bitmap->planes[plane] + bitmap->bplSize - 1;

  custom->bltapt = bpl;
  custom->bltdpt = bpl;
  custom->bltamod = 0;
  custom->bltdmod = 0;
  custom->bltcon0 = (SRCA | DEST) | A_TO_D;
  custom->bltcon1 = BLITREVERSE | FILL_OR;
  custom->bltafwm = -1;
  custom->bltalwm = -1;
  custom->bltsize = (bitmap->height << 6) + (bitmap->width >> 4);
}

void BlitterCopySync(BitmapT *dst, UWORD dstbpl, UWORD x, UWORD y,
                     BitmapT *src, UWORD srcbpl) 
{
  APTR srcbpt = (APTR)src->planes[srcbpl];
  APTR dstbpt = (APTR)dst->planes[dstbpl] + ((x & ~15) >> 3) + y * dst->width / 8;

  WaitBlitter();

  if (x & 15) {
    custom->bltadat = 0xffff;
    custom->bltbpt = srcbpt;
    custom->bltcpt = dstbpt;
    custom->bltdpt = dstbpt;
    custom->bltbmod = -2;
    custom->bltcmod = (dst->width - src->width - 16) >> 3;
    custom->bltdmod = (dst->width - src->width - 16) >> 3;
    custom->bltcon0 = (SRCB | SRCC | DEST) | (ABC | NABC | ABNC | NANBC) | ((x & 15) << ASHIFTSHIFT);
    custom->bltcon1 = ((x & 15) << BSHIFTSHIFT);
    custom->bltalwm = 0;
    custom->bltafwm = -1;
    custom->bltsize = (src->height << 6) + ((src->width + 16) >> 4);
  } else {
    custom->bltapt = srcbpt;
    custom->bltdpt = dstbpt;
    custom->bltamod = 0;
    custom->bltdmod = (dst->width - src->width) >> 3;
    custom->bltcon0 = (SRCA | DEST) | A_TO_D;
    custom->bltcon1 = 0;
    custom->bltalwm = -1;
    custom->bltafwm = -1;
    custom->bltsize = (src->height << 6) + (src->width >> 4);
  }
}

void BlitterCopyAreaSync(BitmapT *dst, UWORD dstbpl,
                         UWORD dx, UWORD dy,
                         BitmapT *src, UWORD srcbpl,
                         UWORD sx, UWORD sy, UWORD sw, UWORD sh)
{
  APTR srcbpt = (APTR)src->planes[srcbpl] + ((sx & ~15) >> 3) + sy * src->width / 8;
  APTR dstbpt = (APTR)dst->planes[dstbpl] + ((dx & ~15) >> 3) + dy * dst->width / 8;

  WaitBlitter();

  /* sx and sw must be multiply of 16 */

  if (0) { //(dx & 15) {
    custom->bltadat = 0xffff;
    custom->bltbpt = srcbpt;
    custom->bltcpt = dstbpt;
    custom->bltdpt = dstbpt;
    custom->bltbmod = (src->width - sw - 16) >> 3;
    custom->bltcmod = (dst->width - sw - 16) >> 3;
    custom->bltdmod = (dst->width - sw - 16) >> 3;
    custom->bltcon0 = (SRCB | SRCC | DEST) | (ABC | NABC | ABNC | NANBC) | ((dx & 15) << ASHIFTSHIFT);
    custom->bltcon1 = ((dx & 15) << BSHIFTSHIFT);
    custom->bltalwm = 0;
    custom->bltafwm = -1;
    custom->bltsize = (sh << 6) + ((sw + 16) >> 4);
  } else {
    custom->bltapt = srcbpt;
    custom->bltdpt = dstbpt;
    custom->bltamod = (src->width - sw) >> 3;
    custom->bltdmod = (dst->width - sw) >> 3;
    custom->bltcon0 = (SRCA | DEST) | A_TO_D;
    custom->bltcon1 = 0;
    custom->bltalwm = -1;
    custom->bltafwm = -1;
    custom->bltsize = (sh << 6) + (sw >> 4);
  }
}

void BlitterSetSync(BitmapT *dst, UWORD dstbpl, UWORD x, UWORD y, UWORD w, UWORD h, UWORD val) {
  APTR dstbpt = (APTR)dst->planes[dstbpl] + ((x & ~15) >> 3) + y * dst->width / 8;

  WaitBlitter();

  if (x & 15) {
    custom->bltadat = 0xffff;
    custom->bltbpt = dstbpt;
    custom->bltcdat = val;
    custom->bltdpt = dstbpt;
    custom->bltbmod = -2;
    custom->bltcmod = (dst->width - w - 16) >> 3;
    custom->bltdmod = (dst->width - w - 16) >> 3;
    custom->bltcon0 = (SRCB | DEST) | (NABC | NABNC | ABC | ANBC) | ((x & 15) << ASHIFTSHIFT);
    custom->bltcon1 = ((x & 15) << BSHIFTSHIFT);
    custom->bltalwm = -1;
    custom->bltafwm = -1;
    custom->bltsize = (h << 6) + ((w + 16) >> 4);
  } else {
    custom->bltadat = val;
    custom->bltdpt = dstbpt;
    custom->bltamod = 0;
    custom->bltdmod = (dst->width - w) >> 3;
    custom->bltcon0 = DEST | A_TO_D;
    custom->bltcon1 = 0;
    custom->bltalwm = -1;
    custom->bltafwm = -1;
    custom->bltsize = (h << 6) + (w >> 4);
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
  line.stride = bitmap->width / 8;
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
