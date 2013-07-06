#define NDEBUG
#include "std/debug.h"
#include "uvmap/common.h"

#ifdef AMIGA
#define LOAD(i) asm("move.w (%1)+,%0" : "+d" (i) : "a>" (data));
#define ADD(i) asm("add.w %1,%0" : "+d" (i) : "a" (offset));

static void RenderOptimizedUVMap(UVMapT *map, PixBufT *canvas) {
  uint16_t *data = (uint16_t *)map->map;
  uint16_t *end = &data[map->width * map->height];
  uint8_t *d = canvas->data;
  uint8_t *t = map->texture->data;
  uint16_t offset = ((map->offsetV & 0xff) << 8) | (map->offsetU & 0xff);

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
static void RenderOptimizedUVMap(UVMapT *map, PixBufT *canvas) {
  uint16_t *data = (uint16_t *)map->map;
  uint16_t *end = &data[map->width * map->height];
  uint8_t *d = canvas->data;
  uint8_t *t = map->texture->data;
  uint16_t offset = ((map->offsetV & 0xff) << 8) | (map->offsetU & 0xff);

  while (data < end)
    *d++ = t[(*data++ + offset) & 0xffff];
}
#endif

static void RenderAccurateUVMap(UVMapT *map, PixBufT *canvas) {
  UV16T *data = (UV16T *)map->map;
  PixBufT *texture = map->texture;
  uint8_t *d = canvas->data;
  size_t i;

  for (i = 0; i < map->width * map->height; i++) {
    Q16T u = data[i].u;
    Q16T v = data[i].v;

    u.integer += map->offsetU;
    v.integer += map->offsetV;

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

void UVMapRender(UVMapT *map, PixBufT *canvas) {
  ASSERT(map->texture, "No texture attached.");

  if (map->type == UV_OPTIMIZED) {
    RenderOptimizedUVMap(map, canvas);
  } else if (map->type == UV_ACCURATE) {
    RenderAccurateUVMap(map, canvas);
  }
}

void UVMapComposeAndRender(PixBufT *canvas, PixBufT *composeMap,
                           UVMapT *map1, UVMapT *map2, int threshold)
{
  uint8_t *cmap = composeMap->data;
  uint8_t *d = canvas->data;
  uint16_t offsetA, offsetB;
  uint8_t *textureA, *textureB;
  uint16_t *mapA, *mapB;
  int i;

  offsetA = ((map1->offsetV & 0xff) << 8) | (map1->offsetU & 0xff);
  offsetB = ((map2->offsetV & 0xff) << 8) | (map2->offsetU & 0xff);

  textureA = map1->texture->data;
  textureB = map2->texture->data;

  mapA = map1->map;
  mapB = map2->map;

  for (i = 0; i < map1->width * map1->height; i++) {
    if (*cmap++ < threshold) {
      *d++ = textureA[(mapA[i] + offsetA) & 0xffff];
    } else {
      *d++ = textureB[(mapB[i] + offsetB) & 0xffff];
    }
  }
}
