#ifndef __BLITTER_H__
#define __BLITTER_H__

#include "gfx.h"

__regargs void BlitterLine(BitmapT *bitmap, UWORD b,
                           UWORD x1, UWORD y1, UWORD x2, UWORD y2);

#endif
