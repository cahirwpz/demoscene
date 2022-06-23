/*
 * The general idea behind this effect is to dynamically manipulate
 * the palette of the pre-rendered grayscale bitmap to achieve
 * the illusion of depth and neon lines turning on and off.
 * Shift registers are also used to move and wrap the image around.
 */

#include <effect.h>
#include <color.h>
#include <copper.h>
#include <fx.h>
#include <gfx.h>

#include <stdlib.h>
#include <system/memory.h>

#define WIDTH 320
#define HEIGHT 256
#define DEPTH 4

static CopListT *coplist[2];
static short active = 0;

static CopInsT *copLine[2][HEIGHT];

/* A struct that controls stripe's colors */
typedef struct {
  short step; /* when transition to the next color in the cycle */
  short orig; /* initial color value from which we start the color cycle */
  short color; /* current stripe's color */
} StripeT;

static StripeT stripe[15];
/* A table for storing colors as they are being shifted */
static short rotated[15];
static u_char shifterValues[16][HEIGHT];

#include "data/stripes.c"
#include "data/floor.c"
/* Width of the leftmost stripe (in pixels) at any given scanline */
#include "data/stripeWidth.c"
/*
 * Light level (values [0-11] where 11 is the darkest) at any given scanline,
 * used to depth to the stripe's colors
 */
#include "data/stripeLight.c"

/*
 * This one is a bit tricky - generate a table of values that will be written
 * to BPLCON1 (PF1Px and PF2Px bits) while shifting the playfields. If you are
 * wondering why this table is so big and has a lot of repeating values - it
 * makes fetching shift values much faster and easier in the ShiftStripes()
 * function that is executed each frame.
 */
static void GenerateShifterValues(void) {
  u_char *data = (u_char *)shifterValues;
  short i, j;
  
  /* for every possible offset */
  for (j = 0; j < 16; j++) {
    /* for each scanline */
    for (i = 0; i < HEIGHT; i++) {
      short s = 1 + ((i * j) >> 8);
      *data++ = (s << 4) | s;
    }
  }
}

static void MakeCopperList(CopListT *cp, short n) {
  short i;

  CopInit(cp);
  CopSetupBitplanes(cp, NULL, &floor, DEPTH);

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
    /* Every stripe starts black */
    s->color = 0;
    s++;
  }
}

static void Init(void) {
  GenerateShifterValues();
  InitStripes();

  SetupMode(MODE_LORES, DEPTH);
  SetupDisplayWindow(MODE_LORES, X(0), Y(0), WIDTH, HEIGHT);
  SetupBitplaneFetch(MODE_LORES, X(-16), WIDTH + 16);
  SetColor(0, 0);

  coplist[0] = NewCopList(100 + 2 * HEIGHT + 15 * HEIGHT / 8);
  coplist[1] = NewCopList(100 + 2 * HEIGHT + 15 * HEIGHT / 8);

  MakeCopperList(coplist[0], 0);
  MakeCopperList(coplist[1], 1);

  CopListActivate(coplist[active ^ 1]);
  EnableDMA(DMAF_RASTER);
}

static void Kill(void) {
  DisableDMA(DMAF_RASTER);
  DeleteCopList(coplist[0]);
  DeleteCopList(coplist[1]);
}

/* Shift colors by an offset */
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

/*
 * Calculate the color of the stripe,
 * taking the light intesivity into consideration.
 */
static void ColorizeStripes(CopInsT **stripeLine) {
  short i;

  for (i = 1; i < 16; i++) {
    CopInsT **line = stripeLine;
    short *light = stripeLight;
    short n = HEIGHT / 8;
    short r, g, b;

    {
      /* Get the offseted color of the stripe */
      short s = rotated[i - 1];

      /* Extract RGB values from color */
      r = s & 0xf00;
      s <<= 4;
      g = s & 0xf00;
      s <<= 4;
      b = s & 0xf00;
    }

    /* Each 8 lines make colors one level brighter */
    while (--n >= 0) {
      /* Set the light value */
      u_char *tab = colortab + (*light);
      /* Write new RGB values back into one variable */
      short color = (tab[r] << 4) | (u_char)(tab[g] | (tab[b] >> 4));

      CopInsSet16(*line + i, color);

      /* Increment the pointers */
      line += 8; light += 8;
    }
  }
}

static void ShiftStripes(CopInsT **line, short offset) {
  short *width = stripeWidth;
  u_char *data = (u_char *)shifterValues;
  u_char *ptr;
  short n = HEIGHT;

  /* Offset the starting point  */
  offset = (offset & 15) << 8;
  data += offset;
  
  while (--n >= 0) {
    /* modify copper instruction that sets bplcon1 */
#if 0
    CopInsSet16(*line++, data[*width++]);
#else
    ptr = (u_char *)(*line++);
    ptr[3] = data[*width++];
#endif
  }
}

static void ControlStripes(void) {
  StripeT *s = stripe;
  short diff = frameCount - lastFrameCount;
  short n = 15;

  while (--n >= 0) {
    /* Decrement the color counter */
    s->step -= diff;
    if (s->step < -128) {
      /* 
       * If we've reached the end of the cycle,
       * start it again at a random point.
       */
      s->step = 64 + (random() & 255);
    }

    if (s->step >= 0) {
      short step = s->step / 8;
      short from, to;

      if (step > 15) {
        /* Make color go brighter */
        from = s->orig;
        to = 0xfff;
        step -= 16;
      } else {
        /* Start going back to the original color */
        from = 0x000;
        to = s->orig;
      }
      s->color = ColorTransition(from, to, step);
    }

    s++;
  }
}

PROFILE(Floor);

static void Render(void) {
  ProfilerStart(Floor);
  {
    short offset = normfx(SIN(frameCount * 8) * 1024) + 1024;
    CopInsT **line = copLine[active];

    ControlStripes();
    ShiftColors(offset);
    ColorizeStripes(line);
    ShiftStripes(line, offset);
  }
  ProfilerStop(Floor);

  CopListRun(coplist[active]);
  TaskWaitVBlank();
  active ^= 1;
}

EFFECT(floor, NULL, NULL, Init, Kill, Render);
