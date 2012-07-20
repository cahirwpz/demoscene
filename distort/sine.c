#include "std/math.h"
#include "std/memory.h"
#include "distort/generate.h"

void GenerateSineDistortion(DistortionMapT *map,
                            size_t xFreq, float xAmp, float xShift,
                            size_t yFreq, float yAmp, float yShift)
{
  float *sinU = CalcSineTable(map->width,
                              xFreq, xAmp / (int)map->textureW, xShift);
  float *sinV = CalcSineTable(map->height,
                              yFreq, yAmp / (int)map->textureH, yShift);

  float du = 1.0f / (int)map->width;
  float dv = 1.0f / (int)map->height;
  float u, v;

  size_t x, y, i;

  for (y = 0, i = 0, v = 0.0f; y < map->height; y++, v += dv)
    for (x = 0, u = 0.0f; x < map->width; x++, u += du, i++)
      DistortionMapSet(map, i, u + sinU[x], v + sinV[y]);

  MemUnref(sinU);
  MemUnref(sinV);
}
