#include <math.h>

#include "hsl.h"

void RGB2HSL(RGB *src, HSL *dst) {
  float r = src->r / 255.0f;
  float g = src->g / 255.0f;
  float b = src->b / 255.0f;

  float hi = max(max(r, g), b);
  float lo = min(min(r, g), b);

  float h = 0.0f;
  float s = 0.0f;
  float l = (lo + hi) / 2.0f;

  if (l > 0.0f) {
    float diff = hi - lo;

    s = diff;

    if (s > 0.0f) {
      float r2, g2, b2;

      s /= (l <= 0.5f) ? (lo + hi) : (2.0f - (lo + hi));

      r2 = (hi - r) / diff;
      g2 = (hi - g) / diff;
      b2 = (hi - b) / diff;

      if (r == hi) {
        h = (g == lo) ? (5.0f + b2) : (1.0f - g2);
      } else if (g == hi) {
        h = (b == lo) ? (1.0f + r2) : (3.0f - b2);
      } else {
        h = (r == lo) ? (3.0f + g2) : (5.0f - r2);
      }

      h /= 6.0f;
    }
  }

  dst->h = h;
  dst->s = s;
  dst->l = l;
}

void HSL2RGB(HSL *src, RGB *dst) {
  float r = 0.0f;
  float g = 0.0f;
  float b = 0.0f;

  float s = src->s;
  float l = src->l;

  float v = (l <= 0.5f) ? (l * (1.0f + s)) : (l + s - l * s);

  if (v > 0) {
    float m = 2.0f * l - v;
    float sv = 1.0f - m / v;

    float h = src->h * 6.0f;

    int sextant = (int)floor(h);

    float fract = h - sextant;
    float vsf = v * sv * fract;
    float mid1 = m + vsf;
    float mid2 = v - vsf;

    switch (sextant) {
      case 0:
        r = v;
        g = mid1;
        b = m;
        break;

      case 1:
        r = mid2;
        g = v;
        b = m;
        break;

      case 2:
        r = m;
        g = v;
        b = mid1;
        break;

      case 3:
        r = m;
        g = mid2;
        b = v;
        break;

      case 4:
        r = mid1;
        g = m;
        b = v;
        break;

      case 5:
        r = v;
        g = m;
        b = mid2;
        break;
    }
  } else {
    r = l;
    g = l;
    b = l;
  }

  dst->r = (uint8_t)(r * 255.0f);
  dst->g = (uint8_t)(g * 255.0f);
  dst->b = (uint8_t)(b * 255.0f);
}
