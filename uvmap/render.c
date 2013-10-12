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

static void RenderAccurateUVMap(UVMapT *map, uint8_t *dst asm("a6")) {
  FP16 *mapU = map->map.accurate.u;
  FP16 *mapV = map->map.accurate.v;
  PixBufT *texture = map->texture;
  int16_t offsetU = map->offsetV;
  int16_t offsetV = map->offsetU;
  int16_t textureW = map->textureW;
  int16_t textureH = map->textureH;
  int n = map->width * map->height;

  do {
    FP16 u = *mapU++;
    FP16 v = *mapV++;

    FP16_i(u) += offsetU;
    FP16_f(v) += offsetV;

    if (FP16_i(u) < 0)
      FP16_i(u) += textureW;
    if (FP16_i(u) >= textureW)
      FP16_i(u) -= textureW;

    if (FP16_i(v) < 0)
      FP16_i(v) += textureH;
    if (FP16_i(v) >= textureH)
      FP16_i(v) -= textureH;

    *dst++ = GetFilteredPixel(texture, u, v);
  } while (--n);
}

void UVMapRender(UVMapT *map, PixBufT *canvas) {
  UVMapRendererT renderer = {
    .mapU = map->map.fast.u,
    .mapV = map->map.fast.v,
    .texture = map->texture->data,
    .pixmap = canvas->data,
    .mapSize = map->width * map->height,
    .offset = ((map->offsetU & 255) << 8) | (map->offsetV & 255)
  };

  ASSERT(map->texture, "No texture attached.");

  if (map->type == UV_FAST) {
    RenderFastUVMapOptimized(&renderer);
  } else if (map->type == UV_NORMAL) {
    RenderNormalUVMapOptimized(&renderer);
  } else if (map->type == UV_ACCURATE) {
    RenderAccurateUVMap(map, canvas->data);
  }
}

void UVMapComposeAndRender(UVMapT *map, PixBufT *canvas, PixBufT *composeMap,
                           uint8_t index)
{
  UVMapRendererT renderer = {
    .mapU = map->map.fast.u,
    .mapV = map->map.fast.v,
    .texture = map->texture->data,
    .pixmap = canvas->data,
    .mapSize = map->width * map->height,
    .offset = ((map->offsetU & 255) << 8) | (map->offsetV & 255),
    .colorMap = composeMap->data,
    .colorIndex = index
  };

  ASSERT(map->type == UV_FAST, "Source map must be fast.");

  UVMapComposeAndRenderOptimized(&renderer);
}
