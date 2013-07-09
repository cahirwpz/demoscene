#ifndef __GFX_LAYERS_H__
#define __GFX_LAYERS_H__

#include "gfx/pixbuf.h"

void LayersCompose(PixBufT *dstBuf, PixBufT *composeMap,
                   PixBufT **layers, size_t no_layers);

#endif
