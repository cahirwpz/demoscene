#include "2d.h"

__regargs void Identity2D(View2D *view) {
  view->m00 = 1 << 8;
  view->m01 = 0;
  view->x = 0;

  view->m11 = 1 << 8;
  view->m10 = 0;
  view->y = 0;
}

__regargs void Translate2D(View2D *view, WORD x, WORD y) {
  view->x += x;
  view->y += y;
}

__regargs void Scale2D(View2D *view, WORD sx, WORD sy) {
  view->m00 = (view->m00 * sx) >> 8;
  view->m01 = (view->m01 * sy) >> 8;
  view->m10 = (view->m10 * sx) >> 8;
  view->m11 = (view->m11 * sy) >> 8;
}

__regargs void Rotate2D(View2D *view, WORD a) {
  WORD sin = sincos[a & 0x1ff].sin;
  WORD cos = sincos[a & 0x1ff].cos;

  WORD m00 = (LONG)(view->m00 * cos - view->m01 * sin) >> 8;
  WORD m01 = (LONG)(view->m00 * sin + view->m01 * cos) >> 8;
  WORD m10 = (LONG)(view->m10 * cos - view->m11 * sin) >> 8;
  WORD m11 = (LONG)(view->m10 * sin + view->m11 * cos) >> 8;

  view->m00 = m00;
  view->m01 = m01;
  view->m10 = m10;
  view->m11 = m11;
}

__regargs void Transform2D(View2D *view, PointT *out, PointT *in, UWORD n) {
  WORD *src = (WORD *)in;
  WORD *dst = (WORD *)out;

  while (n--) {
    register WORD x = *src++;
    register WORD y = *src++;

    *dst++ = ((view->m00 * x + view->m01 * y) >> 8) + view->x;
    *dst++ = ((view->m10 * x + view->m11 * y) >> 8) + view->y;
  }
}
