#ifndef __C2P_H__
#define __C2P_H__

#include "std/types.h"

void c2p1x1_8_c5_bm(__reg("a0") uint8_t *chunky, __reg("a1") struct BitMap *bitmap,
                    __reg("d0") uint16_t width, __reg("d1") uint16_t height,
                    __reg("d2") uint16_t offsetX, __reg("d3") uint16_t offsetY);

#endif
