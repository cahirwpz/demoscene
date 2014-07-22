#include "2d.h"

__regargs void Identity2D(Transform2D *t) {
  t->m00 = 1 << 8;
  t->m01 = 0;
  t->m10 = 0;
  t->m11 = 1 << 8;

  t->x = 0;
  t->y = 0;
}

__regargs void Translate2D(Transform2D *t, WORD x, WORD y) {
  t->x += x;
  t->y += y;
}

__regargs void Scale2D(Transform2D *t, WORD sx, WORD sy) {
  t->m00 = (t->m00 * sx) / 256;
  t->m01 = (t->m01 * sy) / 256;
  t->m10 = (t->m10 * sx) / 256;
  t->m11 = (t->m11 * sy) / 256;
}

__regargs void Rotate2D(Transform2D *t, WORD a) {
  WORD sin = sincos[a & 0x1ff].sin;
  WORD cos = sincos[a & 0x1ff].cos;

  WORD m00 = (LONG)(t->m00 * cos - t->m01 * sin) / 256;
  WORD m01 = (LONG)(t->m00 * sin + t->m01 * cos) / 256;
  WORD m10 = (LONG)(t->m10 * cos - t->m11 * sin) / 256;
  WORD m11 = (LONG)(t->m10 * sin + t->m11 * cos) / 256;

  t->m00 = m00;
  t->m01 = m01;
  t->m10 = m10;
  t->m11 = m11;
}

__regargs void Apply2D(Transform2D *t, PointT *out, PointT *in, UWORD n) {
  UWORD i;

  for (i = 0; i < n; i++) {
    out[i].x = ((t->m00 * in[i].x + t->m01 * in[i].y) / 256) + t->x;
    out[i].y = ((t->m10 * in[i].x + t->m11 * in[i].y) / 256) + t->y;
  }
}
