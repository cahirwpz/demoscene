#include "std/debug.h"
#include "std/math.h"
#include "std/memory.h"
#include "system/fileio.h"
#include "distort/common.h"

static void DeleteDistortionMap(DistortionMapT *map) {
  MemUnref(map->map);
}

TYPEDECL(DistortionMapT, (FreeFuncT)DeleteDistortionMap);

DistortionMapT *NewDistortionMap(size_t width, size_t height,
                                 DistortionMapTypeT type,
                                 size_t textureW, size_t textureH)
{
  DistortionMapT *map = NewInstance(DistortionMapT);
  size_t size = 0;

  switch (type) {
    case DMAP_OPTIMIZED:
      size = sizeof(uint16_t);
      break;
    case DMAP_ACCURATE:
      size = sizeof(UV16T);
      break;
  }

  map->map = MemNew(size * width * height);
  map->type = type;
  map->width = width;
  map->height = height;
  map->textureW = textureW;
  map->textureH = textureH;

  return map;
}

typedef struct DiskDistortionMap {
  uint16_t width;
  uint16_t height;
  uint16_t data[0];
} DiskDistortionMapT;

DistortionMapT *NewDistortionMapFromFile(const StrT fileName) {
  DiskDistortionMapT *file = (DiskDistortionMapT *)ReadFileSimple(fileName);

  if (file) {
    DistortionMapT *map =
      NewDistortionMap(file->width, file->height, DMAP_OPTIMIZED, 256, 256);

    LOG("Distortion map '%s' has size (%d,%d).",
        fileName, (int)file->width, (int)file->height);

    memcpy(map->map, file->data,
           (int)file->width * (int)file->height * sizeof(uint16_t));

    MemUnref(file);

    return map;
  }

  return NULL;
}

void DistortionMapWriteToFile(DistortionMapT *map, const StrT fileName) {
  size_t mapLen = sizeof(uint16_t) * map->width * map->height;
  size_t length = sizeof(int16_t) * 2 + mapLen;
  DiskDistortionMapT *diskMap = MemNew(length);

  diskMap->width = map->width;
  diskMap->height = map->height;
  memcpy(diskMap->data, map->map, mapLen);
  WriteFileSimple(fileName, diskMap, length);
}

void DistortionMapSet(DistortionMapT *map asm("a0"), size_t i asm("d0"),
                      float u asm("fp0"), float v asm("fp1"))
{
  u *= (int)map->textureW;
  v *= (int)map->textureH;

  switch (map->type) {
    case DMAP_OPTIMIZED:
      {
        uint16_t *data = (uint16_t *)map->map;

        data[i] = ((lroundf(v) & 0xff) << 8) | (lroundf(u) & 0xff);
      }
      break;
    case DMAP_ACCURATE:
      {
        UV16T *data = (UV16T *)map->map;

        data[i].u = CastFloatQ16(u);
        data[i].v = CastFloatQ16(v);
      }
      break;
  }
}
