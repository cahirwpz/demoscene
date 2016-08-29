#ifndef __BLTOP_H__
#define __BLTOP_H__

#include "blitter.h"

#define BitmapClear(dst) BitmapSetArea((dst), NULL, 0)
#define BitmapClearArea(dst, area) BitmapSetArea((dst), (area), 0)

__regargs void BitmapSetArea(BitmapT *bitmap, Area2D *area, UWORD color);

__regargs void BitmapCopy(BitmapT *dst, UWORD x, UWORD y, BitmapT *src);
__regargs void BitmapCopyFast(BitmapT *dst, UWORD x, UWORD y, BitmapT *src);
void BitmapCopyMasked(BitmapT *dst, UWORD x, UWORD y, BitmapT *src,
                      BitmapT *mask);
void BitmapCopyArea(BitmapT *dst, UWORD dx, UWORD dy, 
                    BitmapT *src, Area2D *area);

void BitmapAddSaturated(BitmapT *dst, WORD dx, WORD dy, BitmapT *src, BitmapT *carry);
__regargs void BitmapDecSaturated(BitmapT *dst_bm, BitmapT *borrow_bm);
__regargs void BitmapIncSaturated(BitmapT *dst_bm, BitmapT *carry_bm);

__regargs BitmapT *BitmapMakeMask(BitmapT *bitmap);

#endif
