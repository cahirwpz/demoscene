#ifndef __UVMAP_COMMON_H__
#define __UVMAP_COMMON_H__

#include "std/types.h"
#include "std/fp16.h"
#include "gfx/pixbuf.h"

typedef struct {
  Q16T u, v;
} UV16T;

typedef enum { UV_OPTIMIZED, UV_ACCURATE } UVMapTypeT;

typedef struct UVMap {
  UVMapTypeT type;

  /* distortion map data */
	size_t width, height;
  PtrT map;

  /* associated texture, its required size, and offset for texturing */
  PixBufT *texture;
  size_t textureW, textureH;
  int offsetU, offsetV;
} UVMapT;

UVMapT *NewUVMap(size_t width, size_t height, UVMapTypeT type,
                 size_t textureW, size_t textureH);

UVMapT *NewUVMapFromFile(const StrT fileName);
void UVMapWriteToFile(UVMapT *map, const StrT fileName);

typedef enum { 
  UV_EXTRACT_U, 
  UV_EXTRACT_V, 
  UV_REPLACE_U, 
  UV_REPLACE_V
} UVMapAccessT;

void UVMapLayers(UVMapT *map, UVMapAccessT access, PixBufT *pixmap);

void UVMapSetTexture(UVMapT *map, PixBufT *texture);
void UVMapSetOffset(UVMapT *map, int offsetU, int offsetV);

void UVMapSet(UVMapT *map asm("a0"), size_t i asm("d0"),
              float u asm("fp0"), float v asm("fp1"));

void UVMapRender(UVMapT *map, PixBufT *canvas);
void UVMapComposeAndRender(PixBufT *canvas, PixBufT *composeMap,
                           UVMapT *map1, UVMapT *map2, int threshold);

#endif
