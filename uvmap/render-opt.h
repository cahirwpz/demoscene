#ifndef __UVMAP_RENDER_OPT_H__
#define __UVMAP_RENDER_OPT_H__

#include "std/types.h"

typedef struct {
  uint32_t mapSize;
  void     *mapU;
  void     *mapV;
  uint8_t  *texture;
  uint8_t  *pixmap;
  uint8_t  *colorMap;
  uint8_t  *lightMap;
  uint16_t offset;
  uint8_t  colorIndex;
} UVMapRendererT;

void RenderFastUVMapOptimized(UVMapRendererT *renderer asm("a6"));
void RenderNormalUVMapOptimized(UVMapRendererT *renderer asm("a6"));
void UVMapComposeAndRenderOptimized(UVMapRendererT *renderer asm("a6"));

#endif
