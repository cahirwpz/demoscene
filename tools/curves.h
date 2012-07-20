#ifndef __TOOLS_CURVES_H__
#define __TOOLS_CURVES_H__

void CurveLineSegment(float t, float x1, float y1, float x2, float y2,
                      float *x, float *y);
void CurveEpitrochoid(float t, float R, float r, float d,
                      float *x, float *y);
void CurveHypotrochoid(float t, float R, float r, float d,
                       float *x, float *y);

#endif
