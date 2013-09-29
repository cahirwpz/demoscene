#ifndef __UVMAP_RENDER_OPT_H__
#define __UVMAP_RENDER_OPT_H__

#include "std/types.h"

void RenderFastUVMapOptimized(uint16_t *mapU asm("a0"),
                              uint16_t *mapV asm("a1"),
                              uint8_t *texture asm("a2"),
                              uint8_t *dst asm("a6"),
                              int n asm("d5"),
                              int offsetU asm("d6"),
                              int offsetV asm("d7"));

void RenderNormalUVMapOptimized(uint16_t *mapU asm("a0"),
                                uint16_t *mapV asm("a1"),
                                uint8_t *texture asm("a2"),
                                uint8_t *dst asm("a6"),
                                int n asm("d5"),
                                int offsetU asm("d6"),
                                int offsetV asm("d7"));

#endif
