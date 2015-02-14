#ifndef __UVMAP_COMMON_H__
#define __UVMAP_COMMON_H__

#include "std/types.h"
#include "std/fp16.h"
#include "gfx/pixbuf.h"

/*
 * In this context following mapping holds:
 * u -> y
 * v -> x
 */

typedef enum { UV_FAST, UV_NORMAL, UV_ACCURATE } UVMapTypeT;

typedef struct UVMap {
  UVMapTypeT type;

  /* distortion map data */
	size_t width, height;

  union {
    struct {
      uint8_t *u, *v;
    } fast;
    struct {
      int16_t *u, *v;
    } normal;
    struct {
      FP16 *u, *v;
    } accurate;
    struct {
      void *u, *v;
    } any;
  } map;

  PixBufT *lightMap;

  /* associated texture, its required size, and offset for texturing */
  PixBufT *texture;
  size_t textureW, textureH;
  int offsetU, offsetV;
} UVMapT;

UVMapT *NewUVMap(size_t width, size_t height, UVMapTypeT type,
                 size_t textureW, size_t textureH);

UVMapT *NewUVMapFromFile(const char *fileName);
void UVMapWriteToFile(UVMapT *map, const char *fileName);

void UVMapSetTexture(UVMapT *map, PixBufT *texture);
void UVMapSetOffset(UVMapT *map, int offsetU, int offsetV);

__regargs void UVMapSet(UVMapT *map, size_t i, float u, float v);

#endif
