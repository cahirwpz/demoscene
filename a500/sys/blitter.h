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

#define C_TO_D (ABC | NABC | ANBC | NANBC)

#define HALF_ADDER ((SRCA | SRCB | DEST) | A_XOR_B)
#define HALF_ADDER_CARRY ((SRCA | SRCB | DEST) | A_AND_B)
#define FULL_ADDER ((SRCA | SRCB | SRCC | DEST) | (NANBC | NABNC | ANBNC | ABC))
#define FULL_ADDER_CARRY ((SRCA | SRCB | SRCC | DEST) | (NABC | ANBC | ABNC | ABC))

#define HALF_SUB ((SRCA | SRCB | DEST) | A_XOR_B)
#define HALF_SUB_BORROW ((SRCA | SRCB | DEST) | (NABC | NABNC))

/* Values for bltcon1. */
#define LINE_SOLID  (LINEMODE)
#define LINE_ONEDOT (LINEMODE | ONEDOT)

extern UWORD FirstWordMask[16];
extern UWORD LastWordMask[16];

__regargs void BlitterCopySetup(BitmapT *dst, UWORD x, UWORD y, BitmapT *src);
__regargs void BlitterCopyStart(WORD dstbpl, WORD srcbpl);

#define BlitterCopy(dst, dstbpl, x, y, src, srcbpl) ({  \
  BlitterCopySetup((dst), (x), (y), (src));             \
  BlitterCopyStart((dstbpl), (srcbpl));                 \
})

#define BlitterFill(bitmap, plane) \
  BlitterFillArea((bitmap), (plane), NULL)

__regargs void BlitterFillArea(BitmapT *bitmap, WORD plane, Area2D *area);

#define BlitterClear(bitmap, plane) \
  BlitterSetArea((bitmap), (plane), NULL, 0)
#define BlitterClearArea(bitmap, plane, area) \
  BlitterSetArea((bitmap), (plane), (area), 0)
#define BlitterSet(bitmap, plane, pattern) \
  BlitterSetArea((bitmap), (plane), NULL, (pattern))

__regargs void BlitterSetAreaSetup(BitmapT *bitmap, Area2D *area);
__regargs void BlitterSetAreaStart(WORD bplnum, UWORD pattern);

#define BlitterSetArea(bm, bplnum, area, pattern) ({    \
  BlitterSetAreaSetup((bm), (area));                    \
  BlitterSetAreaStart((bplnum), (pattern));             \
})

#define BlitterSetMask(bitmap, plane, x, y, mask, pattern) \
  BlitterSetMaskArea((bitmap), (plane), (x), (y), (mask), NULL, (pattern))

void BlitterSetMaskArea(BitmapT *bitmap, WORD plane, UWORD x, UWORD y,
                        BitmapT *mask, Area2D *area, UWORD pattern);

__regargs void BlitterLineSetup(BitmapT *bitmap, UWORD plane, UWORD bltcon0, UWORD bltcon1);
void BlitterLine(WORD x1 asm("d2"), WORD y1 asm("d3"), WORD x2 asm("d4"), WORD y2 asm("d5"));

static inline BOOL BlitterBusy() {
  return custom->dmaconr & DMAF_BLTDONE;
}

static inline void WaitBlitter() {
  asm("1: btst #6,0xdff002\n"
      "   bnes 1b");
}

#endif
