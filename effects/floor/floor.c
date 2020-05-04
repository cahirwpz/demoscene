#include "startup.h"
#include "hardware.h"
#include "copper.h"
#include "gfx.h"
#include "memory.h"
#include "fx.h"
#include "color.h"
#include "random.h"

#define WIDTH 320
#define HEIGHT 256
#define DEPTH 4

#define FAR   4
#define NEAR 16

static CopListT *coplist[2];
static short active = 0;

static CopInsT *copLine[2][HEIGHT];
static short stripeWidth[HEIGHT];
static short stripeLight[HEIGHT];

typedef struct {
  short step, orig, color;
} StripeT;

static StripeT stripe[15];
static short rotated[15];
static u_char table[4096];

#include "data/stripes.c"
#include "data/floor.c"

static void GenerateStripeLight(void) {
  short *light = stripeLight;
  short level = 11;
  short i;

  for (i = 0; i < HEIGHT / 2; i++)
    *light++ = level;
  for (i = 0; i < HEIGHT / 2; i++)
    *light++ = level - (12 * i) / (HEIGHT / 2);
}

static void GenerateStripeWidth(void) {
  short *width = stripeWidth;
  short i;

  for (i = 0; i < HEIGHT / 2; i++)
    *width++ = (FAR << 4);
  for (i = 0; i < HEIGHT / 2; i++)
    *width++ = (FAR << 4) + ((i << 4) * (NEAR - FAR)) / (HEIGHT / 2);
}

static void GenerateTable(void) {
  u_char *data = table;
  short i, j;

  for (j = 0; j < 16; j++) {
    for (i = 0; i < 256; i++) {
      short s = 1 + ((i * j) >> 8);
      *data++ = (s << 4) | s;
    }
  }
}

static void MakeCopperList(CopListT *cp, short n) {
  short i;

  CopInit(cp);
  CopSetupMode(cp, MODE_LORES, DEPTH);
  CopSetupDisplayWindow(cp, MODE_LORES, X(0), Y(0), WIDTH, HEIGHT);
  CopSetupBitplaneFetch(cp, MODE_LORES, X(-16), WIDTH + 16);
  CopSetupBitplanes(cp, NULL, &floor, DEPTH);
  CopSetColor(cp, 0, 0);

  for (i = 0; i < HEIGHT; i++) {
    CopWait(cp, Y(i), 0);
    copLine[n][i] = CopMove16(cp, bplcon1, 0);

    if ((i & 7) == 0) {
      short j;

      for (j = 1; j < 16; j++)
        CopSetColor(cp, j, 0);
    }
  }

  CopEnd(cp);
}

static void InitStripes(void) {
  StripeT *s = stripe;
  short n = 15;

  while (--n >= 0) {
    s->step = -16 * (random() & 7);
    s->orig = stripes_pal.colors[random() & 3];
    s->color = 0;
    s++;
  }
}

static void Init(void) {
  GenerateTable();
  GenerateStripeLight();
  GenerateStripeWidth();
  InitStripes();

  coplist[0] = NewCopList(100 + 2 * HEIGHT + 15 * HEIGHT / 8);
  coplist[1] = NewCopList(100 + 2 * HEIGHT + 15 * HEIGHT / 8);

  MakeCopperList(coplist[0], 0);
  MakeCopperList(coplist[1], 1);

  CopListActivate(coplist[active ^ 1]);
  EnableDMA(DMAF_RASTER);

  SetFrameCounter(0);
}

static void Kill(void) {
  DeleteCopList(coplist[0]);
  DeleteCopList(coplist[1]);
}

static void ShiftColors(short offset) {
  short *dst = rotated;
  short n = 15;
  short i = 0;

  offset = (offset / 16) % 15;

  while (--n >= 0) {
    short c = i++ - offset;
    if (c < 0)
      c += 15;
    *dst++ = stripe[c].color;
  }
}

static __regargs void ColorizeStripes(CopInsT **stripeLine) {
  short i;

  for (i = 1; i < 16; i++) {
    CopInsT **line = stripeLine;
    short *light = stripeLight;
    short n = HEIGHT / 8;
    short r, g, b;

    {
      short s = rotated[i - 1];

      r = s & 0xf00;
      s <<= 4;
      g = s & 0xf00;
      s <<= 4;
      b = s & 0xf00;
    }

    while (--n >= 0) {
      u_char *tab = colortab + (*light);
      short color = (tab[r] << 4) | (u_char)(tab[g] | (tab[b] >> 4));

      CopInsSet16(*line + i, color);

      line += 8; light += 8;
    }
  }
}

static __regargs void ShiftStripes(CopInsT **line, short offset) {
  short *width = stripeWidth;
  u_char *data = table;
  u_char *ptr;
  short n = HEIGHT;

  offset = (offset & 15) << 8;
  data += offset;

  while (--n >= 0) {
    ptr = (u_char *)(*line++);
    ptr[3] = data[*width++];
  }
}

static void ControlStripes(void) {
  StripeT *s = stripe;
  short diff = frameCount - lastFrameCount;
  short n = 15;

  while (--n >= 0) {
    s->step -= diff;
    if (s->step < -128) {
      s->step = 64 + (random() & 255);
    }

    if (s->step >= 0) {
      short step = s->step / 8;
      short from, to;

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

static void Render(void) {
  // int lines = ReadLineCounter();
  {
    short offset = normfx(SIN(frameCount * 8) * 1024) + 1024;
    CopInsT **line = copLine[active];

    ControlStripes();
    ShiftColors(offset);
    ColorizeStripes(line);
    ShiftStripes(line, offset);
  }
  // Log("floor2: %d\n", ReadLineCounter() - lines);

  CopListRun(coplist[active]);
  TaskWaitVBlank();
  active ^= 1;
}

EffectT Effect = { NULL, NULL, Init, Kill, Render };
