#include "startup.h"
#include "hardware.h"
#include "coplist.h"
#include "gfx.h"
#include "ilbm.h"
#include "tga.h"
#include "memory.h"
#include "fx.h"
#include "color.h"
#include "random.h"

#define WIDTH 320
#define HEIGHT 256
#define DEPTH 4

#define FAR   4
#define NEAR 16

static BitmapT *bitmap;
static CopListT *coplist[2];
static WORD active = 0;

static CopInsT *copLine[2][HEIGHT];
static WORD stripeWidth[HEIGHT];
static WORD stripeLight[HEIGHT];
static WORD stripeColor[16];

typedef struct {
  WORD step, orig, color;
} StripeT;

static StripeT stripe[15];
static WORD rotated[15];
static UBYTE table[4096];

static void Load() {
  bitmap = LoadILBMCustom("data/floor.ilbm", BM_DISPLAYABLE);

  {
    PaletteT *pal = LoadPalette("data/floor-stripes.ilbm");
    ConvertPaletteToRGB4(pal, stripeColor, 16);
    DeletePalette(pal);
  }
}

static void UnLoad() {
  DeleteBitmap(bitmap);
}

static void GenerateStripeLight() {
  WORD *light = stripeLight;
  WORD level = 11;
  WORD i;

  for (i = 0; i < HEIGHT / 2; i++)
    *light++ = level;
  for (i = 0; i < HEIGHT / 2; i++)
    *light++ = level - (12 * i) / (HEIGHT / 2);
}

static void GenerateStripeWidth() {
  WORD *width = stripeWidth;
  WORD i;

  for (i = 0; i < HEIGHT / 2; i++)
    *width++ = (FAR << 4);
  for (i = 0; i < HEIGHT / 2; i++)
    *width++ = (FAR << 4) + ((i << 4) * (NEAR - FAR)) / (HEIGHT / 2);
}

static void GenerateTable() {
  UBYTE *data = table;
  WORD i, j;

  for (j = 0; j < 16; j++) {
    for (i = 0; i < 256; i++) {
      WORD s = 1 + ((i * j) >> 8);
      *data++ = (s << 4) | s;
    }
  }
}

static void MakeCopperList(CopListT *cp, WORD n) {
  WORD i;

  CopInit(cp);
  CopSetupMode(cp, MODE_LORES, DEPTH);
  CopSetupDisplayWindow(cp, MODE_LORES, X(0), Y(0), WIDTH, HEIGHT);
  CopSetupBitplaneFetch(cp, MODE_LORES, X(-16), WIDTH + 16);
  CopSetupBitplanes(cp, NULL, bitmap, DEPTH);
  CopSetRGB(cp, 0, 0);

  for (i = 0; i < HEIGHT; i++) {
    CopWait(cp, Y(i), 0);
    copLine[n][i] = CopMove16(cp, bplcon1, 0);

    if ((i & 7) == 0) {
      WORD j;

      for (j = 1; j < 16; j++)
        CopSetRGB(cp, j, 0);
    }
  }

  CopEnd(cp);
}

static void InitStripes() {
  StripeT *s = stripe;
  WORD n = 15;

  while (--n >= 0) {
    s->step = -16 * (random() & 7);
    s->orig = stripeColor[random() & 3];
    s->color = 0;
    s++;
  }
}

static void Init() {
  GenerateTable();
  GenerateStripeLight();
  GenerateStripeWidth();
  InitStripes();

  coplist[0] = NewCopList(100 + 2 * HEIGHT + 15 * HEIGHT / 8);
  coplist[1] = NewCopList(100 + 2 * HEIGHT + 15 * HEIGHT / 8);

  MakeCopperList(coplist[0], 0);
  MakeCopperList(coplist[1], 1);

  CopListActivate(coplist[active ^ 1]);
  custom->dmacon = DMAF_SETCLR | DMAF_RASTER;

  SetFrameCounter(0);
}

static void Kill() {
  DeleteCopList(coplist[0]);
  DeleteCopList(coplist[1]);
}

static void ShiftColors(WORD offset) {
  WORD *dst = rotated;
  WORD n = 15;
  WORD i = 0;

  offset = (offset / 16) % 15;

  while (--n >= 0) {
    WORD c = i++ - offset;
    if (c < 0)
      c += 15;
    *dst++ = stripe[c].color;
  }
}

static __regargs void ColorizeStripes(CopInsT **stripeLine) {
  WORD i;

  for (i = 1; i < 16; i++) {
    CopInsT **line = stripeLine;
    WORD *light = stripeLight;
    WORD n = HEIGHT / 8;
    WORD r, g, b;

    {
      WORD s = rotated[i - 1];

      r = s & 0xf00;
      s <<= 4;
      g = s & 0xf00;
      s <<= 4;
      b = s & 0xf00;
    }

    while (--n >= 0) {
      UBYTE *tab = colortab + (*light);
      WORD color = (tab[r] << 4) | (UBYTE)(tab[g] | (tab[b] >> 4));

      CopInsSet16(*line + i, color);

      line += 8; light += 8;
    }
  }
}

static __regargs void ShiftStripes(CopInsT **line, WORD offset) {
  WORD *width = stripeWidth;
  UBYTE *data = table;
  UBYTE *ptr;
  WORD n = HEIGHT;

  offset = (offset & 15) << 8;
  data += offset;

  while (--n >= 0) {
    ptr = (UBYTE *)(*line++);
    ptr[3] = data[*width++];
  }
}

static void ControlStripes() {
  StripeT *s = stripe;
  WORD diff = frameCount - lastFrameCount;
  WORD n = 15;

  while (--n >= 0) {
    s->step -= diff;
    if (s->step < -128) {
      s->step = 64 + (random() & 255);
    }

    if (s->step >= 0) {
      WORD step = s->step / 8;
      WORD from, to;

      if (step > 15) {
        from = s->orig;
        to = 0xfff;
        step -= 16;
      } else {
        from = 0x000;
        to = s->orig;
      }
      s->color = ColorTransition(from, to, step);
    }

    s++;
  }
}

static void Render() {
  // LONG lines = ReadLineCounter();
  {
    WORD offset = normfx(SIN(frameCount * 8) * 1024) + 1024;
    CopInsT **line = copLine[active];

    ControlStripes();
    ShiftColors(offset);
    ColorizeStripes(line);
    ShiftStripes(line, offset);
  }
  // Log("floor2: %ld\n", ReadLineCounter() - lines);

  CopListActivate(coplist[active]);
  active ^= 1;
}

EffectT Effect = { Load, UnLoad, Init, Kill, Render };
