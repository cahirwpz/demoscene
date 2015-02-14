#include "std/debug.h"
#include "gfx/layers.h"

static void
LayersComposeLoop(uint8_t *map asm("a3"), uint8_t *dst asm("a1"), uint8_t **src asm("a2"),
                  int n asm("d5"), int no_layers asm("d3"), int bg asm("d4"))
{
  register int i asm("d0");

  for (i = 0; i < n; i++) {
    register int l asm("d1") = *map++;

    if (l < no_layers) 
      *dst++ = src[l][i];
    else
      *dst++ = bg;
  }
}

void LayersCompose(PixBufT *canvas, PixBufT *composeMap,
                   PixBufT **layers, size_t no_layers)
{
  uint8_t **src = alloca(sizeof(uint8_t *) * no_layers);
  int i;

  for (i = 0; i < no_layers; i++) {
    ASSERT(layers[i]->width == canvas->width,
           "Width does not match (%d != %d) for layer #%d.",
           layers[i]->width, canvas->width, i);
    ASSERT(layers[i]->height == canvas->height,
           "Height does not match (%d != %d) for layer #%d.",
           layers[i]->height, canvas->height, i);
    src[i] = layers[i]->data;
  }

  LayersComposeLoop(composeMap->data, canvas->data, src,
                    canvas->width * canvas->height, no_layers,
                    canvas->baseColor);
}
