#include "std/debug.h"
#include "uvmap/render.h"

static void RenderFastUVMap(UVMapT *map, PixBufT *canvas) {
  uint8_t *mapU = map->map.fast.u;
  uint8_t *mapV = map->map.fast.v;
  uint8_t *texture = map->texture->data;
  uint8_t *dst = canvas->data;
  int offsetU = map->offsetU;
  int offsetV = map->offsetV;
  int n = map->width * map->height;

  do {
    uint8_t u = *mapU++ + offsetU;
    uint8_t v = *mapV++ + offsetV;
    *dst++ = texture[u << 8 | v];
  } while (--n);
}

static void RenderAccurateUVMap(UVMapT *map, PixBufT *canvas) {
  Q16T *mapU = map->map.accurate.u;
  Q16T *mapV = map->map.accurate.v;
  PixBufT *texture = map->texture;
  uint8_t *dst = canvas->data;
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

  if (map->type == UV_FAST) {
    RenderFastUVMap(map, canvas);
  } else if (map->type == UV_ACCURATE) {
    RenderAccurateUVMap(map, canvas);
  }
}

void UVMapComposeAndRender(PixBufT *canvas, PixBufT *composeMap,
                           UVMapT *map1, UVMapT *map2)
{
  uint8_t *cmap = composeMap->data;
  uint8_t *dst = canvas->data;
  int i;

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
