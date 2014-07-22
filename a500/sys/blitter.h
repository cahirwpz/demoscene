#ifndef __BLITTER_H__
#define __BLITTER_H__

#include "gfx.h"
#include "hardware.h"

#define LINE_OR   (ABC | ABNC | NABC | NANBC)
#define LINE_EOR  (ABNC | NABC | NANBC)

__regargs void BlitterClear(BitmapT *bitmap, UWORD plane);
__regargs void BlitterFill(BitmapT *bitmap, UWORD plane);
void BlitterLine(BitmapT *bitmap, UWORD plane, UWORD bltcon0, UWORD bltcon1,
                 UWORD x1, UWORD y1, UWORD x2, UWORD y2);

static inline BOOL BlitterBusy() {
  return custom->dmaconr & DMAF_BLTDONE;
}

static inline void WaitBlitter() {
  while (custom->dmaconr & DMAF_BLTDONE);
}

#endif
