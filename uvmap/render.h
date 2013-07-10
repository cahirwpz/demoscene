#ifndef __UVMAP_RENDER_H__
#define __UVMAP_RENDER_H__

#include "uvmap/common.h"

void UVMapRender(UVMapT *map, PixBufT *canvas);
void UVMapComposeAndRender(PixBufT *canvas, PixBufT *composeMap,
                           UVMapT *map1, UVMapT *map2);

#endif
