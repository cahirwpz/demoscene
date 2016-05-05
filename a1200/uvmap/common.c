#include "std/debug.h"
#include "std/math.h"
#include "std/memory.h"
#include "system/rwops.h"
#include "uvmap/common.h"

static void DeleteUVMap(UVMapT *map) {
  if (map->type == UV_FAST) {
    MemUnref(map->map.fast.u);
    MemUnref(map->map.fast.v);
  } else if (map->type == UV_NORMAL) {
    MemUnref(map->map.normal.u);
    MemUnref(map->map.normal.v);
  } else if (map->type == UV_ACCURATE) {
    MemUnref(map->map.accurate.u);
    MemUnref(map->map.accurate.v);
  }
}

TYPEDECL(UVMapT, (FreeFuncT)DeleteUVMap);

UVMapT *NewUVMap(size_t width, size_t height, UVMapTypeT type,
                 size_t textureW, size_t textureH)
{
  UVMapT *map = NewInstance(UVMapT);

  map->type = type;
  map->width = width;
  map->height = height;

  if (type == UV_FAST) {
    ASSERT(textureW == 256 && textureH == 256,
           "In optimized mode texture size has to be 256x256.");
    map->map.fast.u = NewTable(uint8_t, width * height);
    map->map.fast.v = NewTable(uint8_t, width * height);
  } else if (type == UV_NORMAL) {
    map->map.normal.u = NewTable(int16_t, width * height);
    map->map.normal.v = NewTable(int16_t, width * height);
  } else if (type == UV_ACCURATE) {
    map->map.accurate.u = NewTable(FP16, width * height);
    map->map.accurate.v = NewTable(FP16, width * height);
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

typedef struct DiskUVMap {
  uint16_t width;
  uint16_t height;
  uint8_t  data[0];
} DiskUVMapT;

UVMapT *NewUVMapFromFile(const char *fileName) {
  DiskUVMapT *file = (DiskUVMapT *)ReadFileSimple(fileName);

  if (file) {
    UVMapT *map = NewUVMap(file->width, file->height, UV_FAST, 256, 256);

    LOG("Distortion map '%s' has size (%d,%d).",
        fileName, (int)file->width, (int)file->height);

    {
      uint8_t *dstU = map->map.fast.u;
      uint8_t *dstV = map->map.fast.v;
      uint8_t *src = file->data;
      int n = map->width * map->height;

      do {
        *dstU++ = *src++;
        *dstV++ = *src++;
      } while (--n);
    }

    MemUnref(file);

    return map;
  }

  return NULL;
}

void UVMapWriteToFile(UVMapT *map, const char *fileName) {
  size_t mapLen = sizeof(uint16_t) * map->width * map->height;
  size_t length = sizeof(int16_t) * 2 + mapLen;
  DiskUVMapT *diskMap = MemNew(length);

  diskMap->width = map->width;
  diskMap->height = map->height;

  {
    uint8_t *srcU = map->map.fast.u;
    uint8_t *srcV = map->map.fast.v;
    uint8_t *dst = diskMap->data;

    do {
      *dst++ = *srcU++;
      *dst++ = *srcV++;
    } while (--mapLen);
  }

  WriteFileSimple(fileName, diskMap, length);
}

__regargs void UVMapSet(UVMapT *map, size_t i, float u, float v) {
  u *= (int)map->textureW;
  v *= (int)map->textureH;

  if (map->type == UV_FAST) {
    map->map.fast.u[i] = (uint8_t)lroundf(u) & 0xff;
    map->map.fast.v[i] = (uint8_t)lroundf(v) & 0xff;
  } else if (map->type == UV_NORMAL) {
    map->map.normal.u[i] = (int16_t)lroundf(u);
    map->map.normal.v[i] = (int16_t)lroundf(v);
  } else if (map->type == UV_ACCURATE) {
    map->map.accurate.u[i] = FP16_float(u);
    map->map.accurate.v[i] = FP16_float(v);
  }
}
