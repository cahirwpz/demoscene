#include "std/math.h"
#include "gfx/colors.h"
#include "gfx/hsl.h"

typedef struct ColorAcc {
  int r, g, b, n;
} ColorAccT;

void ColorsInvert(ColorT *dst, ColorT *src, size_t count) {
  size_t i;

  for (i = 0; i < count; i++) {
    dst[i].r = ~src[i].r;
    dst[i].g = ~src[i].g;
    dst[i].b = ~src[i].b;
  }
}

void ColorsFindMinMax(ColorT *colors, size_t count,
                      ColorAccT *cmin, ColorAccT *cmax)
{
  size_t i;

  cmax->r = cmin->r = colors[0].r;
  cmax->g = cmin->g = colors[0].g;
  cmax->b = cmin->b = colors[0].b;
  cmax->n = cmin->n = 1;

  for (i = 1; i < count; i++) {
    int r = colors[i].r;
    int g = colors[i].g;
    int b = colors[i].b;

    if (r < cmin->r) cmin->r = r;
    if (r > cmax->r) cmax->r = r;
    if (g < cmin->g) cmin->g = g;
    if (g > cmax->g) cmax->g = g;
    if (b < cmin->b) cmin->b = b;
    if (b > cmax->b) cmax->b = b;
  }
}

void ColorsCalcAvg(ColorT *colors, size_t count, ColorAccT *cavg) {
  size_t i;

  cavg->r = cavg->g = cavg->b = 0;

  for (i = 0; i < count; i++) {
    cavg->r += colors[i].r;
    cavg->g += colors[i].g;
    cavg->b += colors[i].b;
  }

  cavg->r /= count;
  cavg->g /= count;
  cavg->b /= count;
}

void ColorsContrast(ColorT *dst, ColorT *src, size_t count) {
  size_t i;
  ColorAccT cmin, cmax, cdinv;

  ColorsFindMinMax(src, count, &cmin, &cmax);

  cdinv.r = 65536 / (cmax.r - cmin.r);
  cdinv.g = 65536 / (cmax.g - cmin.g);
  cdinv.b = 65536 / (cmax.b - cmin.b);

  for (i = 0; i < count; i++) {
    dst[i].r = ((src[i].r - cmin.r) * cdinv.r) >> 8;
    dst[i].g = ((src[i].g - cmin.g) * cdinv.g) >> 8;
    dst[i].b = ((src[i].b - cmin.b) * cdinv.b) >> 8;
  }
}

void ColorsAverage(uint8_t *dst, ColorT *src, size_t count) {
  size_t i;

  /* Conversion according to CCIR601 standard. */
  const ColorAccT weight = { 0.299f * 65536.0f,
                             0.587f * 65536.0f,
                             0.114f * 65536.0f };

  for (i = 0; i < count; i++) {
    uint8_t value = (weight.r * src[i].r +
                     weight.g * src[i].g +
                     weight.b * src[i].b) >> 16;

    dst[i] = value;
  }
}

/*
 * @param change: all coefficient have to be in range [-1.0, 1.0f]
 */
void ColorsChangeHSL(ColorT *dst, ColorT *src, size_t count, ColorVectorT *d) {
  size_t i;

  if (d->s > 0.0f)
    d->s = 1.0f / (1.0f - d->s);
  else
    d->s += 1.0f;

  if (d->l > 0.0f)
    d->l = 1.0f / (1.0f - d->l);
  else
    d->l += 1.0f;

  for (i = 0; i < count; i++) {
    ColorVectorT hsl;

    RGB2HSL(&src[i], &hsl);

    hsl.h += d->h;
    hsl.s *= d->s;
    hsl.l *= d->l;

    if (hsl.h > 1.0f)
      hsl.h -= 1.0f;
    else if (hsl.h < 0.0f)
      hsl.h += 1.0f;

    if (hsl.s > 1.0f)
      hsl.s = 1.0f;

    if (hsl.l > 1.0f)
      hsl.l = 1.0f;

    HSL2RGB(&hsl, &dst[i]);
	}
}
