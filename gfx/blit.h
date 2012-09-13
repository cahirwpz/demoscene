#ifndef __GFX_BLIT_H__
#define __GFX_BLIT_H__

#include "gfx/common.h"
#include "gfx/pixbuf.h"

void PixBufBlit(PixBufT *dstBuf, int x, int y,
                PixBufT *srcBuf, const RectT *srcRect);

void PixBufBlitScaled(PixBufT *dstBuf, size_t x, size_t y, int w, int h,
                      PixBufT *srcBuf);

#endif
