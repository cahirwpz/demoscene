#include "std/memory.h"
#include "uvmap/scaling.h"

static inline Q16T Div8(Q16T x) {
  asm("asrl #3,%0"
      : "+d" (x)
      : "d" (x));
  return x;
}

static UV16T *StepperFromUVMap(UVMapT *map asm("a0")) {
  int size = map->width * (map->height - 1);
  UV16T *stepper = NewTable(UV16T, size);

  {
    Q16T *row1 = (Q16T *)map->map;
    Q16T *row2 = ((Q16T *)map->map) + map->width * 2;
    Q16T *data = (Q16T *)stepper;

    do {
      *data++ = Div8(SubQ16(*row2++, *row1++));
      *data++ = Div8(SubQ16(*row2++, *row1++));
    } while (--size);
  }

  return stepper;
}

static void ScaleLine8x(uint16_t *dstUV asm("a0"), UV16T *srcUV asm("a1"), size_t w asm("d0")) {
  uint8_t *dst = (uint8_t *)dstUV;
  Q16T *src = (Q16T *)srcUV;

  do {
    Q16T u = *src++;
    Q16T v = *src++;
    Q16T du = Div8(SubQ16(src[0], u));
    Q16T dv = Div8(SubQ16(src[1], v));

    int i = 7;

    do {
      *dst++ = v.integer;
      v = AddQ16(v, dv);
      *dst++ = u.integer;
      u = AddQ16(u, du);
    } while (--i >= 0);
  } while (--w > 1);
}

static void IncrementUV(UV16T *uv asm("a0"), UV16T *duv asm("a1"), size_t w asm("d0")) {
  Q16T *dst = (Q16T *)uv;
  Q16T *src = (Q16T *)duv;

  w *= 2;

  do {
    IAddQ16(dst++, *src++);
  } while (--w);
}

void UVMapScale8x(UVMapT *dstMap, UVMapT *srcMap) {
  UV16T *stepper = StepperFromUVMap(srcMap);
  UV16T *src = (UV16T *)srcMap->map;
  UV16T *duv = stepper;
  uint16_t *dst = (uint16_t *)dstMap->map;

  size_t srcRow = srcMap->width;
  size_t dstRow = dstMap->width;

  size_t i = srcMap->height - 1;

  do {
    size_t j = 8;

    do {
      ScaleLine8x(dst, src, srcRow);
      IncrementUV(src, duv, srcRow);

      dst += dstRow;
    } while (--j);

    duv += srcRow;
    src += srcRow;
  } while (--i);

  MemUnref(stepper);
}
