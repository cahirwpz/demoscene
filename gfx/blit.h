#ifndef __GFX_BLIT_H__
#define __GFX_BLIT_H__

#include "gfx/pixbuf.h"

void PixBufBlit(PixBufT *dstBuf, size_t x, size_t y, PixBufT *srcBuf);
void PixBufBlitScaled(PixBufT *dstBuf, size_t x, size_t y, int w, int h,
                      PixBufT *srcBuf);

#endif
