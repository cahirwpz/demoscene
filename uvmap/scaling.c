#include "std/debug.h"
#include "std/memory.h"
#include "uvmap/scaling.h"

int UVMapExpanderThreshold = 224;

static const Q16T PLUS = { 256, 0 };
static const Q16T MINUS = { -256, 0 };

static inline Q16T Div8(Q16T x) {
  asm("asrl #3,%0"
      : "+d" (x)
      : "d" (x));
  return x;
}
 
__regargs static void
StepperFromMap(Q16T *map, Q16T *stepper, const int width, const int height) {
  int n = width * (height - 1);
  Q16T *row1 = map;
  Q16T *row2 = map + width;

  do {
    Q16T diff = SubQ16(*row2++, *row1++);

    if (diff.integer > UVMapExpanderThreshold)
      IAddQ16(&diff, MINUS);
    if (diff.integer < -UVMapExpanderThreshold)
      IAddQ16(&diff, PLUS);

    *stepper++ = Div8(diff);
  } while (--n);
}

__regargs static void
FastStepperFromMap(Q16T *map, Q16T *stepper, const int width, const int height) {
  int n = width * (height - 1);
  Q16T *row1 = map;
  Q16T *row2 = map + width;

  do {
    *stepper++ = Div8(SubQ16(*row2++, *row1++));
  } while (--n);
}

__regargs static void
ExpandLine8x(int16_t *dst, Q16T *src, int width) {
  do {
    Q16T x0 = *src++;
    Q16T x1, x2, x3, x4, x5, x6, x7;
    Q16T dx = SubQ16(*src, x0);

    if (dx.integer > UVMapExpanderThreshold)
      IAddQ16(&dx, MINUS);
    if (dx.integer < -UVMapExpanderThreshold)
      IAddQ16(&dx, PLUS);

    dx = Div8(dx);

    x1 = AddQ16(x0, dx);
    x2 = AddQ16(x1, dx);
    x3 = AddQ16(x2, dx);
    x4 = AddQ16(x3, dx);
    x5 = AddQ16(x4, dx);
    x6 = AddQ16(x5, dx);
    x7 = AddQ16(x6, dx);

    *dst++ = x0.integer;
    *dst++ = x1.integer;
    *dst++ = x2.integer;
    *dst++ = x3.integer;
    *dst++ = x4.integer;
    *dst++ = x5.integer;
    *dst++ = x6.integer;
    *dst++ = x7.integer;
  } while (--width);
}

__regargs static void
FastExpandLine8x(int16_t *dst, Q16T *src, int width) {
  do {
    Q16T x0 = *src++;
    Q16T x1, x2, x3, x4, x5, x6, x7;
    Q16T dx = Div8(SubQ16(*src, x0));

    x1 = AddQ16(x0, dx);
    x2 = AddQ16(x1, dx);
    x3 = AddQ16(x2, dx);
    x4 = AddQ16(x3, dx);
    x5 = AddQ16(x4, dx);
    x6 = AddQ16(x5, dx);
    x7 = AddQ16(x6, dx);

    *dst++ = x0.integer;
    *dst++ = x1.integer;
    *dst++ = x2.integer;
    *dst++ = x3.integer;
    *dst++ = x4.integer;
    *dst++ = x5.integer;
    *dst++ = x6.integer;
    *dst++ = x7.integer;
  } while (--width);
}

__regargs static void
Increment(Q16T *x, Q16T *dx, int width) {
  do {
    IAddQ16(x++, *dx++);
  } while (--width);
}

__regargs static void
MapExpand8x(int16_t *dst, const int dwidth, Q16T *stepper,
            Q16T *src, const int width, const int height)
{
  int i = height - 1;

  StepperFromMap(src, stepper, width, height);

  do {
    int j = 8;

    do {
      ExpandLine8x(dst, src, width - 1);
      Increment(src, stepper, width);

      dst += dwidth;
    } while (--j);

    stepper += width;
    src += width;
  } while (--i);
}

__regargs static void
FastMapExpand8x(int16_t *dst, const int dwidth, Q16T *stepper,
                Q16T *src, const int width, const int height)
{
  int i = height - 1;

  FastStepperFromMap(src, stepper, width, height);

  do {
    int j = 8;

    do {
      FastExpandLine8x(dst, src, width - 1);
      Increment(src, stepper, width);

      dst += dwidth;
    } while (--j);

    stepper += width;
    src += width;
  } while (--i);
}

void UVMapScale8x(UVMapT *dstMap, UVMapT *srcMap) {
  Q16T *stepper = NewTable(Q16T, srcMap->width * (srcMap->height - 1));

  ASSERT(srcMap->type == UV_ACCURATE, "Source map must be accurate.");
  ASSERT(dstMap->type == UV_NORMAL, "Destination map must be normal.");

  MapExpand8x(dstMap->map.normal.u, dstMap->width, stepper,
              srcMap->map.accurate.u, srcMap->width, srcMap->height);
  FastMapExpand8x(dstMap->map.normal.v, dstMap->width, stepper,
                  srcMap->map.accurate.v, srcMap->width, srcMap->height);

  MemUnref(stepper);
}
