#ifndef __GFX_BLIT_H__
#define __GFX_BLIT_H__

#include "gfx/pixbuf.h"

void PixBufBlitTransparent(PixBufT *dstBuf asm("a0"), size_t x asm("d0"),
                           size_t y asm("d1"), PixBufT *srcBuf asm("a1"));

#endif
