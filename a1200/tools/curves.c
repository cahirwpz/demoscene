#include "std/math.h"

void CurveLineSegment(float t, float x1, float y1, float x2, float y2,
                      float *x, float *y)
{
  t = fmod(t, 2.0);

  if (t < 0)
    t += 2.0f;

  if (t >= 1.0f)
    t = 2.0f - t;

  *x = (1.0f - t) * x1 + t * x2;
  *y = (1.0f - t) * y1 + t * y2;
}

void CurveEpitrochoid(float t, float R, float r, float d,
                      float *x, float *y)
{
  float alpha = 2.0f * M_PI * t;
  float a = R + r;
  float b = a / r;

  *x = a * cos(alpha) - d * r * cos(b * alpha);
  *y = a * sin(alpha) - d * r * sin(b * alpha);
}

void CurveHypotrochoid(float t, float R, float r, float d,
                       float *x, float *y)
{
  float alpha = 2.0f * M_PI * t;
  float a = R - r;
  float b = a / r;

  *x = a * cos(alpha) + d * r * cos(b * alpha);
  *y = a * sin(alpha) - d * r * sin(b * alpha);
}
