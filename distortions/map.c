#include "std/memory.h"
#include "distortions/map.h"

static void DeleteDistortionMap(DistortionMapT *map) {
  MemUnref(map->map);
}

DistortionMapT *NewDistortionMap(size_t width, size_t height,
                                 DistortionMapTypeT type,
                                 size_t textureW, size_t textureH)
{
  DistortionMapT *map = NewRecordGC(DistortionMapT, (FreeFuncT)DeleteDistortionMap);
  size_t size = 0;

  switch (type) {
    case DMAP_OPTIMIZED:
      size = sizeof(uint16_t);
      break;
    case DMAP_ACCURATE:
      size = sizeof(TextureUV_Q16T);
      break;
  }

  map->width = width;
  map->height = height;
  map->textureW = textureW;
  map->textureH = textureH;
  map->map = MemNew0(size * width * height, NULL);

  return map;
}

void DistortionMapSet(DistortionMapT *map, size_t i, Q16T u, Q16T v) {
  switch (map->type) {
    case DMAP_OPTIMIZED:
      {
        uint16_t *data = (uint16_t *)map->map;

        data[i] = ((IntRoundQ16(v) & 0xff) << 8) | (IntRoundQ16(u) & 0xff);
      }
      break;
    case DMAP_ACCURATE:
      {
        TextureUV_Q16T *data = (TextureUV_Q16T *)map->map;

        data[i].u = u;
        data[i].v = v;
      }
      break;
  }
}
