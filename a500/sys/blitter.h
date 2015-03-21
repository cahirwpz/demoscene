#ifndef __BLITTER_H__
#define __BLITTER_H__

#include "gfx.h"
#include "2d.h"
#include "hardware.h"

/* Values for bltcon0. */
#define LINE_OR  ((ABC | ABNC | NABC | NANBC) | (SRCA | SRCC | DEST))
#define LINE_EOR ((ABNC | NABC | NANBC) | (SRCA | SRCC | DEST))

#define A_XOR_B (ANBC | NABC | ANBNC | NABNC)
#define A_AND_B (ABC | ABNC)
#define A_AND_NOT_B (ANBC | ANBNC)

#define HALF_ADDER ((SRCA | SRCB | DEST) | A_XOR_B)
#define HALF_ADDER_CARRY ((SRCA | SRCB | DEST) | A_AND_B)
#define FULL_ADDER ((SRCA | SRCB | SRCC | DEST) | (NANBC | NABNC | ANBNC | ABC))
#define FULL_ADDER_CARRY ((SRCA | SRCB | SRCC | DEST) | (NABC | ANBC | ABNC | ABC))

#define HALF_SUB ((SRCA | SRCB | DEST) | A_XOR_B)
#define HALF_SUB_BORROW ((SRCA | SRCB | DEST) | (NABC | NABNC))

/* Values for bltcon1. */
#define LINE_SOLID  (LINEMODE)
#define LINE_ONEDOT (LINEMODE | ONEDOT)

#define BlitterFill(bitmap, plane) \
  BlitterFillArea((bitmap), (plane), NULL)

__regargs void BlitterFillArea(BitmapT *bitmap, WORD plane, Area2D *area);

#define BlitterClear(bitmap, plane) \
  BlitterSetArea((bitmap), (plane), NULL, 0)
#define BlitterClearArea(bitmap, plane, area) \
  BlitterSetArea((bitmap), (plane), (area), 0)
#define BlitterSet(bitmap, plane, pattern) \
  BlitterSetArea((bitmap), (plane), NULL, (pattern))

__regargs void BlitterSetArea(BitmapT *bitmap, WORD plane, Area2D *area, UWORD pattern);

void BlitterSetMaskArea(BitmapT *bitmap, WORD plane, UWORD x, UWORD y,
                        BitmapT *mask, Area2D *area, UWORD pattern);

__regargs void BlitterLineSetup(BitmapT *bitmap, UWORD plane, UWORD bltcon0, UWORD bltcon1);
__regargs void BlitterLine(WORD x1, WORD y1, WORD x2, WORD y2);

static inline BOOL BlitterBusy() {
  return custom->dmaconr & DMAF_BLTDONE;
}

static inline void WaitBlitter() {
  asm("1: btst #6,0xdff002\n"
      "   bnes 1b");
}

#endif
