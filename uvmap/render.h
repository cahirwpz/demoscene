#ifndef __UVMAP_RENDER_H__
#define __UVMAP_RENDER_H__

#include "uvmap/common.h"

void UVMapRender(UVMapT *map, PixBufT *canvas);
void UVMapComposeAndRender(UVMapT *map, PixBufT *canvas, PixBufT *composeMap,
                           uint8_t index);

#endif
