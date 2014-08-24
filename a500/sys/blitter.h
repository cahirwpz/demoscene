#ifndef __BLITTER_H__
#define __BLITTER_H__

#include "gfx.h"
#include "hardware.h"

/* Values for bltcon0. */
#define LINE_OR  ((ABC | ABNC | NABC | NANBC) | (SRCA | SRCC | DEST))
#define LINE_EOR ((ABNC | NABC | NANBC) | (SRCA | SRCC | DEST))

/* Values for bltcon1. */
#define LINE_SOLID  (LINEMODE)
#define LINE_ONEDOT (LINEMODE | ONEDOT)

__regargs void BlitterClear(BitmapT *bitmap, UWORD plane);
__regargs void BlitterFill(BitmapT *bitmap, UWORD plane);
__regargs void BlitterLine(BitmapT *bitmap, UWORD plane,
                           UWORD bltcon0, UWORD bltcon1, Line2D *line);

static inline BOOL BlitterBusy() {
  return custom->dmaconr & DMAF_BLTDONE;
}

static inline void WaitBlitter() {
  asm("1: btst #6,0xdff002\n"
      "   bnes 1b");
}

#endif
