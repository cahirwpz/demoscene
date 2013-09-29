#include "std/debug.h"
#include "uvmap/render.h"
#include "uvmap/render-opt.h"

#if 0
static void RenderFastUVMap(UVMapT *map, uint8_t *dst asm("a6")) {
  uint8_t *mapU = map->map.fast.u;
  uint8_t *mapV = map->map.fast.v;
  uint8_t *texture = map->texture->data;
  uint8_t offsetU = map->offsetU;
  uint8_t offsetV = map->offsetV;
  int n = map->width * map->height;

  do {
    uint8_t u = *mapU++ + offsetU;
    uint8_t v = *mapV++ + offsetV;
    *dst++ = texture[u << 8 | v];
  } while (--n);
}

static void RenderNormalUVMap(UVMapT *map, uint8_t *dst asm("a6")) {
  int16_t *mapU = map->map.normal.u;
  int16_t *mapV = map->map.normal.v;
  uint8_t *texture = map->texture->data;
  int16_t offsetU = map->offsetU;
  int16_t offsetV = map->offsetV;
  int n = map->width * map->height;

  do {
    int16_t u = *mapU++ + offsetU;
    int16_t v = *mapV++ + offsetV;
    *dst++ = texture[(uint8_t)u << 8 | (uint8_t)v];
  } while (--n);
}
#endif

void RenderFastUVMap(UVMapT *map, uint8_t *dst asm("a6")) {
  RenderFastUVMapOptimized(map->map.normal.u,
                           map->map.normal.v,
                           map->texture->data,
                           dst,
                           map->width * map->height,
                           map->offsetU,
                           map->offsetV);
}


void RenderNormalUVMap(UVMapT *map, uint8_t *dst asm("a6")) {
  RenderNormalUVMapOptimized(map->map.normal.u,
                             map->map.normal.v,
                             map->texture->data,
                             dst,
                             map->width * map->height,
                             map->offsetU,
                             map->offsetV);
}

static void RenderAccurateUVMap(UVMapT *map, uint8_t *dst asm("a6")) {
  Q16T *mapU = map->map.accurate.u;
  Q16T *mapV = map->map.accurate.v;
  PixBufT *texture = map->texture;
  int16_t offsetU = map->offsetV;
  int16_t offsetV = map->offsetU;
  int16_t textureW = map->textureW;
  int16_t textureH = map->textureH;
  int n = map->width * map->height;

  do {
    Q16T u = *mapU++;
    Q16T v = *mapV++;

    u.integer += offsetU;
    v.integer += offsetV;

    if (u.integer < 0)
      u.integer += textureW;
    if (u.integer >= textureW)
      u.integer -= textureW;

    if (v.integer < 0)
      v.integer += textureH;
    if (v.integer >= textureH)
      v.integer -= textureH;

    *dst++ = GetFilteredPixel(texture, u, v);
  } while (--n);
}

void UVMapRender(UVMapT *map, PixBufT *canvas) {
  ASSERT(map->texture, "No texture attached.");

  if (map->type == UV_FAST)
    RenderFastUVMap(map, canvas->data);
  else if (map->type == UV_NORMAL)
    RenderNormalUVMap(map, canvas->data);
  else if (map->type == UV_ACCURATE)
    RenderAccurateUVMap(map, canvas->data);
}

void UVMapComposeAndRender(PixBufT *canvas, PixBufT *composeMap,
                           UVMapT *map1, UVMapT *map2)
{
  uint8_t *cmap = composeMap->data;
  uint8_t *dst = canvas->data;
  int i;

  ASSERT((map1->type == UV_FAST) && (map2->type == UV_FAST),
         "Both source maps must be fast.");

  for (i = 0; i < map1->width * map1->height; i++) {
    uint8_t *texture;
    uint8_t u, v;

    if (*cmap++ == 0) {
      texture = map1->texture->data;
      u = map1->map.fast.u[i] + map1->offsetU;
      v = map1->map.fast.v[i] + map1->offsetV;
    } else {
      texture = map2->texture->data;
      u = map2->map.fast.u[i] + map2->offsetU;
      v = map2->map.fast.v[i] + map2->offsetV;
    }

    *dst++ = texture[(u << 8) | v];
  }
}
