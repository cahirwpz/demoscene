#ifndef __BLITTER_H__
#define __BLITTER_H__

#include <gfx.h>
#include <2d.h>
#include <custom.h>

/* definitions for blitter control register 0 */
#define ABC __BIT(7)
#define ABNC __BIT(6)
#define ANBC __BIT(5)
#define ANBNC __BIT(4)
#define NABC __BIT(3)
#define NABNC __BIT(2)
#define NANBC __BIT(1)
#define NANBNC __BIT(0)

#define DEST __BIT(8)
#define SRCC __BIT(9)
#define SRCB __BIT(10)
#define SRCA __BIT(11)

#define ASHIFT(x) (((x) & 15) << 12)
#define BSHIFT(x) (((x) & 15) << 12)

/* definitions for blitter control register 1 */
#define LINEMODE __BIT(0)

/* bltcon1 in normal mode */
#define OVFLAG __BIT(5)
#define FILL_XOR __BIT(4)
#define FILL_OR __BIT(3)
#define FILL_CARRYIN __BIT(2)
#define BLITREVERSE __BIT(1)

/* bltcon1 in line mode */
#define SIGNFLAG __BIT(6)
#define SUD __BIT(4)
#define SUL __BIT(3)
#define AUL __BIT(2)
#define ONEDOT __BIT(1)

/* some commonly used operations */
#define A_AND_B (ABC | ABNC)
#define A_AND_NOT_B (ANBC | ANBNC)
#define NOT_A_AND_B (NABC | NABNC)
#define A_OR_B (ABC | ANBC | NABC | ABNC | ANBNC | NABNC)
#define A_OR_C (ABC | NABC | ABNC | ANBC | NANBC | ANBNC)
#define A_TO_D (ABC | ANBC | ABNC | ANBNC)
#define A_XOR_B (ANBC | NABC | ANBNC | NABNC)
#define A_XOR_C (NABC | ABNC | NANBC | ANBNC)
#define C_TO_D (ABC | NABC | ANBC | NANBC)

#define HALF_ADDER ((SRCA | SRCB | DEST) | A_XOR_B)
#define HALF_ADDER_CARRY ((SRCA | SRCB | DEST) | A_AND_B)
#define FULL_ADDER ((SRCA | SRCB | SRCC | DEST) | (NANBC | NABNC | ANBNC | ABC))
#define FULL_ADDER_CARRY                                                       \
  ((SRCA | SRCB | SRCC | DEST) | (NABC | ANBC | ABNC | ABC))

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
extern const u_short FirstWordMask[16];
extern const u_short LastWordMask[16];
extern const u_short LineMode[4][2];

/* Common blitter macros. */
static inline bool BlitterBusy(void) {
  return custom->dmaconr & DMAF_BLTDONE;
}

static inline void _WaitBlitter(CustomPtrT custom_) {
  asm("1: btst #6,%0@(2)\n" /* dmaconr */
      "   bnes 1b"
      :: "a" (custom_));
}

#define WaitBlitter() _WaitBlitter(custom)

/* @brief Stops Blitter activity gracefully. */
void BlitterStop(void);

/* Blitter copy. */
void BlitterCopySetup(const BitmapT *dst, u_short x, u_short y,
                      const BitmapT *src);
void BlitterCopyStart(short dstbpl, short srcbpl);

#define BlitterCopy(dst, dstbpl, x, y, src, srcbpl) ({  \
  BlitterCopySetup((dst), (x), (y), (src));             \
  BlitterCopyStart((dstbpl), (srcbpl));                 \
})

/* Blitter copy area. */
void BlitterCopyAreaSetup(const BitmapT *dst, u_short x, u_short y,
                          const BitmapT *src, const Area2D *area);
void BlitterCopyAreaStart(short dstbpl, short srcbpl);

#define BlitterCopyArea(dst, dstbpl, x, y, src, srcbpl, area) ({        \
  BlitterCopyAreaSetup((dst), (x), (y), (src), (area));                 \
  BlitterCopyAreaStart((dstbpl), (srcbpl));                             \
})

/* Blitter copy fast. */
void BlitterCopyFastSetup(const BitmapT *dst, u_short x, u_short y,
                          const BitmapT *src);
void BlitterCopyFastStart(short dstbpl, short srcbpl);

/* Blitter copy masked. */
void BlitterCopyMaskedSetup(const BitmapT *dst, u_short x, u_short y,
                            const BitmapT *src, const BitmapT *msk);
void BlitterCopyMaskedStart(short dstbpl, short srcbpl);

/* Bitmap copy. */
void BitmapCopy(const BitmapT *dst, u_short x, u_short y, const BitmapT *src);
void BitmapCopyFast(const BitmapT *dst, u_short x, u_short y,
                    const BitmapT *src);
void BitmapCopyMasked(const BitmapT *dst, u_short x, u_short y,
                      const BitmapT *src, const BitmapT *mask);
void BitmapCopyArea(const BitmapT *dst, u_short dx, u_short dy, 
                    const BitmapT *src, const Area2D *area);

/* Blitter or operation. */
/* TODO: does not behave correctly for unaligned `x` */
void BlitterOrSetup(const BitmapT *dst, u_short x, u_short y,
                      const BitmapT *src);
void BlitterOrStart(short dstbpl, short srcbpl);

#define BlitterOr(dst, dstbpl, x, y, src, srcbpl) ({  \
  BlitterOrSetup((dst), (x), (y), (src));             \
  BlitterOrStart((dstbpl), (srcbpl));                 \
})

/* Blitter fill. */
void BlitterFillArea(const BitmapT *bitmap, short plane, const Area2D *area);

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
void BlitterSetAreaStart(short bplnum, u_short pattern);

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

void BitmapSetArea(const BitmapT *bitmap, const Area2D *area, u_short color);

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

void BitmapDecSaturated(const BitmapT *dst_bm, const BitmapT *borrow_bm);
void BitmapIncSaturated(const BitmapT *dst_bm, const BitmapT *carry_bm);

BitmapT *BitmapMakeMask(const BitmapT *bitmap);

#endif
