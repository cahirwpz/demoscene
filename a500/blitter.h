#ifndef __BLITTER_H__
#define __BLITTER_H__

#include "gfx.h"
#include "hardware.h"

__regargs void BlitterLine(BitmapT *bitmap, UWORD b,
                           UWORD x1, UWORD y1, UWORD x2, UWORD y2);

static inline BOOL BlitterBusy() {
  return custom->dmaconr & DMAF_BLTDONE;
}

static inline void WaitBlitter() {
  while (custom->dmaconr & DMAF_BLTDONE);
}

#endif
