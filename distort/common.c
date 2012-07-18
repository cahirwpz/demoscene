#include "std/math.h"
#include "std/memory.h"
#include "distort/common.h"

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

void DistortionMapSet(DistortionMapT *map asm("a0"), size_t i asm("d0"),
                      float u asm("fp0"), float v asm("fp1"))
{
  u *= map->textureW;
  v *= map->textureH;

  switch (map->type) {
    case DMAP_OPTIMIZED:
      {
        uint16_t *data = (uint16_t *)map->map;

        data[i] = ((lroundf(v) & 0xff) << 8) | (lroundf(u) & 0xff);
      }
      break;
    case DMAP_ACCURATE:
      {
        TextureUV_Q16T *data = (TextureUV_Q16T *)map->map;

        data[i].u = CastFloatQ16(u);
        data[i].v = CastFloatQ16(v);
      }
      break;
  }
}
