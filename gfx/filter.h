#ifndef __GFX_BLUR_H__
#define __GFX_BLUR_H__

#include "gfx/pixbuf.h"

void BlurH3(PixBufT *dstBuf, PixBufT *srcBuf);
void BlurV3(PixBufT *dstBuf, PixBufT *srcBuf);

#endif
