#include "std/array.h"
#include "std/memory.h"

#include "gfx/colors.h"
#include "gfx/quantize.h"

typedef struct Color {
  int r, g, b;
} ColorT;

static size_t ColorFindMaxComponent(ColorT *color, int *value) {
  int v = color->r;
  int i = 0;

  if (v < color->g) {
    v = color->g;
    i = 1;
  }

  if (v < color->b) {
    v = color->b;
    i = 2;
  }

  *value = v;

  return i;
}

typedef struct ColorBox {
  ArrayT *colors;

  size_t begin;
  size_t end;
  size_t count;

  ColorT average;
  ColorT color;
  ColorT variance;

  size_t axis;
  int weight;
} ColorBoxT;

static void ColorBoxCalcAverage(ColorBoxT *box);
static void ColorBoxCalcVariance(ColorBoxT *box);

ColorBoxT *NewColorBox(ArrayT *colors, size_t begin, size_t end, ColorT *average) {
  ColorBoxT *box = NewRecord(ColorBoxT);

  box->colors = colors;
  box->begin  = begin;
  box->end    = end;

  if (average) {
    box->average = *average;
  } else {
    ColorBoxCalcAverage(box);
  }
    
  box->count = end - begin + 1;

  box->color.r = box->average.r / box->count;
  box->color.g = box->average.g / box->count;
  box->color.b = box->average.b / box->count;

  ColorBoxCalcVariance(box);

  box->axis = ColorFindMaxComponent(&box->variance, &box->weight);

  return box;
}

static void ColorBoxCalcAverage(ColorBoxT *box) {
  void Add(RGB *color) {
    box->average.r += (int)color->r;
    box->average.g += (int)color->b;
    box->average.b += (int)color->b;
  }

  ArrayForEachInRange(box->colors, box->begin, box->end, (IterFuncT)Add, NULL);
}

static void ColorBoxCalcVariance(ColorBoxT *box) {
  void AddSqrDiff(RGB *color) {
    ColorT diff = {
      (int)color->r - box->color.r,
      (int)color->g - box->color.g,
      (int)color->b - box->color.b
    };

    box->variance.r += diff.r * diff.r;
    box->variance.g += diff.g * diff.g;
    box->variance.b += diff.b * diff.b;
  }

  ArrayForEachInRange(box->colors, box->begin, box->end, (IterFuncT)AddSqrDiff, NULL);
}

void ColorBoxSplit(ColorBoxT *box, ColorBoxT **leftBoxPtr, ColorBoxT **rightBoxPtr) {
  size_t axis = box->axis;
  size_t i;

  int median = ((int *)&box->color)[axis];

  CmpT CompareColor(RGB *color) {
    uint8_t c = ((uint8_t *)color)[axis];

    if (c < median)
      return CMP_LT;
    else if (c > median)
      return CMP_GT;
    else
      return CMP_EQ;
  }

  i = ArrayPartition(box->colors, box->begin, box->end, NULL);

  {
    ColorBoxT *leftBox = NewColorBox(box->colors, box->begin, i, NULL);
    ColorBoxT *rightBox;

    ColorT rightAverage = {
      box->average.r - leftBox->average.r,
      box->average.g - leftBox->average.b,
      box->average.b - leftBox->average.b
    };

    rightBox = NewColorBox(box->colors, i + 1, box->end, &rightAverage);

    *leftBoxPtr = leftBox;
    *rightBoxPtr = rightBox;
  }
}
