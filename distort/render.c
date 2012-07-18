#define NDEBUG
#include "std/debug.h"
#include "distort/common.h"

#ifdef AMIGA
void RenderDistortionOptimized(DistortionMapT *map asm("a0"),
                               CanvasT *canvas asm("a1"),
                               PixBufT *texture asm("a2"),
                               int offsetU asm("d0"),
                               int offsetV asm("d1"));
#else
void RenderDistortionsOptimized(DistortionMapT *map,
                                CanvasT *canvas, PixBufT *texture,
                                int offsetU, int offsetV)
{
  uint16_t *data = (uint16_t *)map->map;
  uint8_t *d = GetCanvasPixelData(canvas);
  uint8_t *t = texture->data;
  size_t i;

  offsetU <<= 8;

  for (i = 0; i < map->width * map->height; i++) {
    size_t uv = data[i];
    size_t index = ((uv + offsetV) & 0xff) | ((uv + offsetU) & 0xff00);

    d[i] = t[index];
  }
}
#endif

void RenderDistortionAccurate(DistortionMapT *map,
                              CanvasT *canvas, PixBufT *texture,
                              int offsetU, int offsetV)
{
  TextureUV_Q16T *data = (TextureUV_Q16T *)map->map;
  uint8_t *d = GetCanvasPixelData(canvas);
  size_t i;

  for (i = 0; i < map->width * map->height; i++) {
    Q16T u = data[i].u;
    Q16T v = data[i].v;

    u.integer += offsetU;
    v.integer += offsetV;

    if (u.integer < 0)
      u.integer += texture->width;
    if (u.integer >= texture->width)
      u.integer -= texture->width;

    if (v.integer < 0)
      v.integer += texture->height;
    if (v.integer >= texture->height)
      v.integer -= texture->height;

    d[i] = GetFilteredPixel(texture, data[i].u, data[i].v);
  }
}

void RenderDistortion(DistortionMapT *map, CanvasT *canvas, PixBufT *texture,
                      int offsetU, int offsetV)
{
  ASSERT(texture->width == map->textureW, "Texture width mismatch %d != %d.",
         (int)texture->width, (int)map->textureW);
  ASSERT(texture->height == map->textureH, "Texture height mismatch %d != %d.",
         (int)texture->height, (int)map->textureH);
  ASSERT((offsetU < -texture->width) || (offsetU > texture->width),
         "Offset U out of allowed range.");
  ASSERT((offsetV < -texture->height) || (offsetV > texture->height),
         "Offset V out of allowed range.");

  switch (map->type) {
    case DMAP_OPTIMIZED:
      ASSERT(texture->width == texture->height == 256,
             "In optimized mode texture size has to be 256x256.");
      RenderDistortionOptimized(map, canvas, texture, offsetU, offsetV);
      break;
    case DMAP_ACCURATE:
      RenderDistortionAccurate(map, canvas, texture, offsetU, offsetV);
      break;
  }
}
