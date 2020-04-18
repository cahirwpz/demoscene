#ifndef __BLITTER_H__
#define __BLITTER_H__

#include "gfx.h"
#include "2d.h"
#include "hardware.h"

/* Values for bltcon0. */
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

#define BC0F_LINE_OR ((ABC | ABNC | NABC | NANBC) | (SRCA | SRCC | DEST))
#define BC0F_LINE_EOR ((ABNC | NABC | NANBC) | (SRCA | SRCC | DEST))

/* Line drawing modes. */
#define LINE_OR 0
#define LINE_EOR 1
#define LINE_SOLID 0
#define LINE_ONEDOT 2

/* Precalculated masks for bltafwm and bltalwm registers. */
extern u_short FirstWordMask[16];
extern u_short LastWordMask[16];
extern u_short LineMode[4][2];

/* Common blitter macros. */
static inline bool BlitterBusy(void) {
  return custom->dmaconr & DMAF_BLTDONE;
}

static inline void _WaitBlitter(CustomPtrT custom) {
  asm("1: btst #6,%0@(2)\n" /* dmaconr */
      "   bnes 1b"
      :: "a" (custom));
}

#define WaitBlitter() _WaitBlitter(custom)

/* Blitter copy. */
void BlitterCopySetup(const BitmapT *dst, u_short x, u_short y,
                      const BitmapT *src);
__regargs void BlitterCopyStart(short dstbpl, short srcbpl);

#define BlitterCopy(dst, dstbpl, x, y, src, srcbpl) ({  \
  BlitterCopySetup((dst), (x), (y), (src));             \
  BlitterCopyStart((dstbpl), (srcbpl));                 \
})

/* Blitter copy area. */
void BlitterCopyAreaSetup(const BitmapT *dst, u_short x, u_short y,
                          const BitmapT *src, const Area2D *area);
__regargs void BlitterCopyAreaStart(short dstbpl, short srcbpl);

#define BlitterCopyArea(dst, dstbpl, x, y, src, srcbpl, area) ({        \
  BlitterCopyAreaSetup((dst), (x), (y), (src), (area));                 \
  BlitterCopyAreaStart((dstbpl), (srcbpl));                             \
})

/* Bitmap copy. */
__regargs void BitmapCopy(const BitmapT *dst, u_short x, u_short y,
                          const BitmapT *src);
__regargs void BitmapCopyFast(const BitmapT *dst, u_short x, u_short y,
                              const BitmapT *src);
void BitmapCopyMasked(const BitmapT *dst, u_short x, u_short y,
                      const BitmapT *src, const BitmapT *mask);
void BitmapCopyArea(const BitmapT *dst, u_short dx, u_short dy, 
                    const BitmapT *src, const Area2D *area);

/* Blitter fill. */
__regargs void BlitterFillArea(const BitmapT *bitmap, short plane,
                               const Area2D *area);

#define BlitterFill(bitmap, plane) \
  BlitterFillArea((bitmap), (plane), NULL)

/* Blitter clear. */
#define BlitterClear(bitmap, plane) \
  BlitterSetArea((bitmap), (plane), NULL, 0)
#define BlitterClearArea(bitmap, plane, area) \
  BlitterSetArea((bitmap), (plane), (area), 0)

#define BitmapClear(dst) \
  BitmapSetArea((dst), NULL, 0)
#define BitmapClearArea(dst, area) \
  BitmapSetArea((dst), (area), 0)

/* Blitter set. */
void BlitterSetAreaSetup(const BitmapT *bitmap, const Area2D *area);
__regargs void BlitterSetAreaStart(short bplnum, u_short pattern);

void BlitterSetMaskArea(const BitmapT *bitmap, short plane, u_short x, u_short y,
                        const BitmapT *mask, const Area2D *area, u_short pattern);

#define BlitterSet(bitmap, plane, pattern) \
  BlitterSetArea((bitmap), (plane), NULL, (pattern))

#define BlitterSetArea(bm, bplnum, area, pattern) ({    \
  BlitterSetAreaSetup((bm), (area));                    \
  BlitterSetAreaStart((bplnum), (pattern));             \
})

#define BlitterSetMask(bitmap, plane, x, y, mask, pattern) \
  BlitterSetMaskArea((bitmap), (plane), (x), (y), (mask), NULL, (pattern))

/* Bitmap set. */

__regargs void BitmapSetArea(const BitmapT *bitmap,
                             const Area2D *area, u_short color);

/* Blitter line. */
#define BlitterLineSetup(bitmap, plane, mode) \
  BlitterLineSetupFull((bitmap), (plane), (mode), -1)

void BlitterLineSetupFull(const BitmapT *bitmap, u_short plane,
                          u_short mode, u_short pattern);
void BlitterLine(short x1 asm("d2"), short y1 asm("d3"),
                 short x2 asm("d4"), short y2 asm("d5"));

/* Other operations. */
void BitmapAddSaturated(const BitmapT *dst, short dx, short dy,
                        const BitmapT *src, const BitmapT *carry);

__regargs void BitmapDecSaturated(const BitmapT *dst_bm,
                                  const BitmapT *borrow_bm);
__regargs void BitmapIncSaturated(const BitmapT *dst_bm,
                                  const BitmapT *carry_bm);

__regargs BitmapT *BitmapMakeMask(const BitmapT *bitmap);

#endif
