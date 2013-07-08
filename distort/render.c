#define NDEBUG
#include "std/debug.h"
#include "distort/common.h"

#ifdef AMIGA
#define LOAD(i) asm("move.w (%1)+,%0" : "+d" (i) : "a>" (data));
#define ADD(i) asm("add.w %1,%0" : "+d" (i) : "a" (offset));

void RenderDistortionOptimized(DistortionMapT *map,
                               PixBufT *canvas, PixBufT *texture,
                               int offsetU, int offsetV)
{
  uint16_t *data = (uint16_t *)map->map;
  uint16_t *end = &data[map->width * map->height];
  uint8_t *d = canvas->data;
  uint8_t *t = texture->data;
  uint16_t offset = ((offsetV & 0xff) << 8) | (offsetU & 0xff);

  uint32_t i0 = 0;
  uint32_t i1 = 0;
  uint32_t i2 = 0;
  uint32_t i3 = 0;
  uint32_t i4 = 0;
  uint32_t i5 = 0;
  uint32_t i6 = 0;
  uint32_t i7 = 0;

  while (data < end) {
    LOAD(i0);
    LOAD(i1);
    LOAD(i2);
    LOAD(i3);
    LOAD(i4);
    LOAD(i5);
    LOAD(i6);
    LOAD(i7);
    ADD(i0);
    ADD(i1);
    ADD(i2);
    ADD(i3);
    ADD(i4);
    ADD(i5);
    ADD(i6);
    ADD(i7);
    *d++ = t[i0];
    *d++ = t[i1];
    *d++ = t[i2];
    *d++ = t[i3];
    *d++ = t[i4];
    *d++ = t[i5];
    *d++ = t[i6];
    *d++ = t[i7];
  }
}
#else
void RenderDistortionOptimized(DistortionMapT *map,
                               PixBufT *canvas, PixBufT *texture,
                               int offsetU, int offsetV)
{
  uint16_t *data = (uint16_t *)map->map;
  uint16_t *end = &data[map->width * map->height];
  uint8_t *d = canvas->data;
  uint8_t *t = texture->data;
  uint16_t offset = ((offsetV & 0xff) << 8) | (offsetU & 0xff);

  while (data < end)
    *d++ = t[(*data++ + offset) & 0xffff];
}
#endif

void RenderDistortionAccurate(DistortionMapT *map,
                              PixBufT *canvas, PixBufT *texture,
                              int offsetU, int offsetV)
{
  UV16T *data = (UV16T *)map->map;
  uint8_t *d = canvas->data;
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

void RenderDistortion(DistortionMapT *map, PixBufT *canvas, PixBufT *texture,
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
