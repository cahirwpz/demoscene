#ifndef __C2P_H__
#define __C2P_H__

#include <graphics/gfx.h>

#include "std/types.h"

void c2p1x1_8_c5_bm(uint8_t *chunky asm("a0"), struct BitMap *bitmap asm("a1"),
                    uint16_t width asm("d0"), uint16_t height asm("d1"),
                    uint16_t offsetX asm("d2"), uint16_t offsetY asm("d3"));

#endif
