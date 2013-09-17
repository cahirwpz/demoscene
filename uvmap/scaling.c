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
ExpandLine8x(uint8_t *dst, Q16T *src, int width) {
  do {
    Q16T x = *src++;
    Q16T dx = SubQ16(*src, x);

    if (dx.integer > UVMapExpanderThreshold)
      IAddQ16(&dx, MINUS);
    if (dx.integer < -UVMapExpanderThreshold)
      IAddQ16(&dx, PLUS);

    dx = Div8(dx);

    *dst++ = x.integer; /* 0 */
    x = AddQ16(x, dx);
    *dst++ = x.integer; /* 1 */
    x = AddQ16(x, dx);
    *dst++ = x.integer; /* 2 */
    x = AddQ16(x, dx);
    *dst++ = x.integer; /* 3 */
    x = AddQ16(x, dx);
    *dst++ = x.integer; /* 4 */
    x = AddQ16(x, dx);
    *dst++ = x.integer; /* 5 */
    x = AddQ16(x, dx);
    *dst++ = x.integer; /* 6 */
    x = AddQ16(x, dx);
    *dst++ = x.integer; /* 7 */
  } while (--width);
}

__regargs static void
Increment(Q16T *x, Q16T *dx, int width) {
  do {
    IAddQ16(x++, *dx++);
  } while (--width);
}

__regargs static void
MapExpand8x(uint8_t *dst, const int dwidth, Q16T *stepper,
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

void UVMapScale8x(UVMapT *dstMap, UVMapT *srcMap) {
  Q16T *stepper = NewTable(Q16T, srcMap->width * (srcMap->height - 1));

  MapExpand8x(dstMap->map.normal.u, dstMap->width, stepper,
              srcMap->map.accurate.u, srcMap->width, srcMap->height);
  MapExpand8x(dstMap->map.normal.v, dstMap->width, stepper,
              srcMap->map.accurate.v, srcMap->width, srcMap->height);

  MemUnref(stepper);
}
