#include "std/math.h"
#include "std/memory.h"
#include "distortions/generate.h"

void GenerateSineDistortion(DistortionMapT *map,
                            size_t xFreq, float xAmp, float xShift,
                            size_t yFreq, float yAmp, float yShift)
{
  float *sinX = CalcSineTable(map->width, xFreq, xAmp, xShift);
  float *sinY = CalcSineTable(map->height, yFreq, yAmp, yShift);

  float dy = 1.0f / (int)map->height;
  float dx = 1.0f / (int)map->width;
  float fx, fy;
  size_t x, y;
  int i;

  for (y = 0, i = 0, fy = 0.0f; y < map->height; y++, fy += dy)
    for (x = 0, fx = 0.0f; x < map->width; x++, fx += dx, i++)
      DistortionMapSet(map, i, fx + sinX[x], fy + sinY[y]);

  MemUnref(sinX);
  MemUnref(sinY);
}
