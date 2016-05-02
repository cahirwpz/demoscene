#include "std/random.h"
#include "std/memory.h"
#include "gfx/pixbuf.h"
#include "gfx/spline.h"

void GeneratePerlinPlasma(PixBufT *pixbuf, size_t step, int32_t *random) {
  size_t size = pixbuf->height / step;
  size_t x, y, i;

  SplineT *colors = NewSpline(size, TRUE);

  for (x = 0; x < pixbuf->width; x += step) {
    void PutDataColumn(PixBufT *pixbuf, size_t index, float *value) {
      PutPixel(pixbuf, x, index * step * pixbuf->width, (uint8_t)*value);
    }

    for (i = 0; i < size; i++)
      colors->knots[i].value = RandomFloat(random) * 255.0f;

    SplineAttachCatmullRomTangents(colors);
    SplineInterpolate(colors, step, pixbuf, (SetItemFuncT)PutDataColumn);
  }

  for (y = 0; y < pixbuf->height; y++) {
    void PutDataRow(PixBufT *pixbuf, size_t index, float *value) {
      PutPixel(pixbuf, index, y, (uint8_t)*value);
    }

    for (i = 0; i < size; i++)
      colors->knots[i].value = GetPixel(pixbuf, i * step, y);

    SplineAttachCatmullRomTangents(colors);
    SplineInterpolate(colors, step, pixbuf, (SetItemFuncT)PutDataRow);
  }

  MemUnref(colors);
}
