#ifndef __UVMAP_RENDER_OPT_H__
#define __UVMAP_RENDER_OPT_H__

#include "std/types.h"

typedef struct {
  void *mapU;
  void *mapV;
  uint8_t *texture;
  uint8_t *dst;
  int n;
  int offsetU;
  int offsetV;
  uint8_t *cmap;
  int index;
} UVMapRenderT;

void RenderFastUVMapOptimized(uint8_t *mapU asm("a0"),
                              uint8_t *mapV asm("a1"),
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

void UVMapComposeAndRenderOptimized(UVMapRenderT *render asm("a6"));

#endif
