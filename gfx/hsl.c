#include "std/math.h"
#include "gfx/hsl.h"

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
    
    s = diff / ((l < 0.5f) ? (hi + lo) : (2.0f - (hi + lo)));

    if (r == hi) {
      h = (g - b) / diff;
    } else if (g == hi) {
      h = 2.0f + (b - r) / diff;
    } else {
      h = 4.0f + (r - g) / diff;
    }

    h /= 6.0f;

    if (h < 0.0f)
      h += 1.0f;
  }

  dst->h = h;
  dst->s = s;
  dst->l = l;
}

void HSL2RGB(HSL *src, RGB *dst) {
  float r = 1.0f;
  float g = 1.0f;
  float b = 1.0f;

  float h = src->h * 6.0f;
  float s = src->s;
  float l = src->l;

  if (s > 0.0f) {
    float v = (l < 0.5f) ? (l * (1.0f + s)) : (l + s - l * s);
    float m = 2.0f * l - v;
    float sv = 1.0f - m / v;

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
  }

  dst->r = (uint8_t)(r * 255.0f);  
  dst->g = (uint8_t)(g * 255.0f);  
  dst->b = (uint8_t)(b * 255.0f);  
}
