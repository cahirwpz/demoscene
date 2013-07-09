#include "std/debug.h"
#include "gfx/layers.h"

void LayersCompose(PixBufT *dstBuf, PixBufT *composeMap,
                   PixBufT **layers, size_t no_layers)
{
  uint8_t *dst = dstBuf->data;
  uint8_t *map = composeMap->data;
  uint8_t bg = dstBuf->baseColor;
  int i;

  for (i = 0; i < no_layers; i++) {
    ASSERT(layers[i]->width == dstBuf->width,
           "Width does not match (%ld != %ld) for layer #%d.",
           layers[i]->width, dstBuf->width, i);
    ASSERT(layers[i]->height == dstBuf->height,
           "Height does not match (%ld != %ld) for layer #%d.",
           layers[i]->height, dstBuf->height, i);
  }

  for (i = 0; i < dstBuf->width * dstBuf->height; i++) {
    int l = *map++;

    if (l < no_layers)
      *dst++ = layers[l]->data[i];
    else
      *dst++ = bg;
  }
}
