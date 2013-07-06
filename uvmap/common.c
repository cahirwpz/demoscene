#include "std/debug.h"
#include "std/math.h"
#include "std/memory.h"
#include "system/fileio.h"
#include "uvmap/common.h"

static void DeleteUVMap(UVMapT *map) {
  MemUnref(map->map);
}

TYPEDECL(UVMapT, (FreeFuncT)DeleteUVMap);

UVMapT *NewUVMap(size_t width, size_t height, UVMapTypeT type,
                 size_t textureW, size_t textureH)
{
  UVMapT *map = NewInstance(UVMapT);

  map->type = type;
  map->width = width;
  map->height = height;

  if (type == UV_OPTIMIZED) {
    ASSERT(textureW == 256 && textureH == 256,
           "In optimized mode texture size has to be 256x256.");
    map->map = NewTable(uint16_t, width * height);
  } else if (type == UV_ACCURATE) {
    map->map = NewTable(UV16T, width * height);
  }

  /* initially there's no texture attached */
  map->textureW = textureW;
  map->textureH = textureH;

  return map;
}

void UVMapSetOffset(UVMapT *map, int offsetU, int offsetV) {
  ASSERT((offsetU < -map->textureW) || (offsetU > map->textureW),
         "Offset U out of allowed range.");
  ASSERT((offsetV < -map->textureH) || (offsetV > map->textureH),
         "Offset V out of allowed range.");

  map->offsetU = offsetU;
  map->offsetV = offsetV;
}

void UVMapSetTexture(UVMapT *map, PixBufT *texture) {
  ASSERT(texture->width == map->textureW, "Texture width mismatch %d != %d.",
         (int)texture->width, (int)map->textureW);
  ASSERT(texture->height == map->textureH, "Texture height mismatch %d != %d.",
         (int)texture->height, (int)map->textureH);
  map->texture = texture;
}

void UVMapLayers(UVMapT *map, UVMapAccessT access, PixBufT *pixbuf)
{
  int n = pixbuf->width * pixbuf->height;
  uint8_t *p = pixbuf->data;
  uint8_t *m = map->map;
  uint8_t offset;

  ASSERT(map->width == pixbuf->width,
         "Width does not match (%ld != %ld).", map->width, pixbuf->width);
  ASSERT(map->height == pixbuf->height,
         "Height does not match (%ld != %ld).", map->height, pixbuf->height);
  ASSERT(map->type == UV_OPTIMIZED,
         "Not implemented for non-optimized maps.");

  switch (access) {
    case UV_EXTRACT_V:
      offset = map->offsetV;
      m++;
      while (n--) {
        *p++ = *m++ + offset;
        m++;
      }
      break;
    case UV_EXTRACT_U:
      offset = map->offsetU;
      while (n--) {
        *p++ = *m++ + offset;
        m++;
      }
      break;
    case UV_REPLACE_V:
      offset = map->offsetV;
      m++;
      while (n--) {
        *m++ = *p++ - offset;
        m++;
      }
      break;
    case UV_REPLACE_U:
      offset = map->offsetU;
      while (n--) {
        *m++ = *p++ - offset;
        m++;
      }
      break;
  }
}

typedef struct DiskUVMap {
  uint16_t width;
  uint16_t height;
  uint16_t data[0];
} DiskUVMapT;

UVMapT *NewUVMapFromFile(const StrT fileName) {
  DiskUVMapT *file = (DiskUVMapT *)ReadFileSimple(fileName);

  if (file) {
    UVMapT *map =
      NewUVMap(file->width, file->height, UV_OPTIMIZED, 256, 256);

    LOG("Distortion map '%s' has size (%d,%d).",
        fileName, (int)file->width, (int)file->height);

    memcpy(map->map, file->data,
           (int)file->width * (int)file->height * sizeof(uint16_t));

    MemUnref(file);

    return map;
  }

  return NULL;
}

void UVMapWriteToFile(UVMapT *map, const StrT fileName) {
  size_t mapLen = sizeof(uint16_t) * map->width * map->height;
  size_t length = sizeof(int16_t) * 2 + mapLen;
  DiskUVMapT *diskMap = MemNew(length);

  diskMap->width = map->width;
  diskMap->height = map->height;
  memcpy(diskMap->data, map->map, mapLen);
  WriteFileSimple(fileName, diskMap, length);
}

void UVMapSet(UVMapT *map asm("a0"), size_t i asm("d0"),
                      float u asm("fp0"), float v asm("fp1"))
{
  u *= (int)map->textureW;
  v *= (int)map->textureH;

  if (map->type == UV_OPTIMIZED) {
    uint16_t *data = (uint16_t *)map->map;

    data[i] = ((lroundf(v) & 0xff) << 8) | (lroundf(u) & 0xff);
  } else if (map->type == UV_ACCURATE) {
    UV16T *data = (UV16T *)map->map;

    data[i].u = CastFloatQ16(u);
    data[i].v = CastFloatQ16(v);
  }
}
