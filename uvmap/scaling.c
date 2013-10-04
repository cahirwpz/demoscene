#include "std/debug.h"
#include "std/memory.h"
#include "uvmap/scaling.h"
#include "uvmap/scaling-opt.h"

int UVMapExpanderThreshold = 224;

#if 0
static const FP16 PLUS = { 256, 0 };
static const FP16 MINUS = { -256, 0 };

static inline FP16 Div8(FP16 x) {
  asm("asrl #3,%0"
      : "+d" (x)
      : "d" (x));
  return x;
}
 
__regargs static void
StepperFromMap(FP16 *map, FP16 *stepper, const int width, const int height) {
  int n = width * (height - 1);
  FP16 *row1 = map;
  FP16 *row2 = map + width;

  do {
    FP16 diff = SubQ16(*row2++, *row1++);

    if (diff.integer > UVMapExpanderThreshold)
      IAddQ16(&diff, MINUS);
    if (diff.integer < -UVMapExpanderThreshold)
      IAddQ16(&diff, PLUS);

    *stepper++ = Div8(diff);
  } while (--n);
}

__regargs static void
FastStepperFromMap(FP16 *map, FP16 *stepper, const int width, const int height) {
  int n = width * (height - 1);
  FP16 *row1 = map;
  FP16 *row2 = map + width;

  do {
    *stepper++ = Div8(SubQ16(*row2++, *row1++));
  } while (--n);
}

__regargs static void
ExpandLine8x(int16_t *dst, FP16 *src, int width) {
  do {
    FP16 x0 = *src++;
    FP16 x1, x2, x3, x4, x5, x6, x7;
    FP16 dx = SubQ16(*src, x0);

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
FastExpandLine8x(int16_t *dst, FP16 *src, int width) {
  do {
    FP16 x0 = *src++;
    FP16 x1, x2, x3, x4, x5, x6, x7;
    FP16 dx = Div8(SubQ16(*src, x0));

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
Increment(FP16 *x, FP16 *dx, int width) {
  do {
    IAddQ16(x++, *dx++);
  } while (--width);
}
#endif

__regargs static void
MapExpand8x(int16_t *dst, const int dwidth, FP16 *stepper,
            FP16 *src, const int width, const int height)
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
FastMapExpand8x(int16_t *dst, const int dwidth, FP16 *stepper,
                FP16 *src, const int width, const int height)
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
  FP16 *stepper = NewTable(FP16, srcMap->width * (srcMap->height - 1));

  ASSERT(srcMap->type == UV_ACCURATE, "Source map must be accurate.");
  ASSERT(dstMap->type == UV_NORMAL, "Destination map must be normal.");

  MapExpand8x(dstMap->map.normal.u, dstMap->width, stepper,
              srcMap->map.accurate.u, srcMap->width, srcMap->height);
  FastMapExpand8x(dstMap->map.normal.v, dstMap->width, stepper,
                  srcMap->map.accurate.v, srcMap->width, srcMap->height);

  MemUnref(stepper);
}
