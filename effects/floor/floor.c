/* ===== floor effect =====
The general idea behind this effect is to dynamically manipulate the palette of the pre-rendered grayscale bitmap to achieve the illusion of depth and neon lines turning on and off. 
Shift registers are also used to move and wrap the image around.
*/

#include "effect.h"
#include "copper.h"
#include "gfx.h"
#include "memory.h"
#include "fx.h"
#include "color.h"
#include <stdlib.h>

#define WIDTH 320
#define HEIGHT 256
#define DEPTH 4

// Width of the stripes (in pixels) at the point farthest from the camera
#define FAR   4
// Width of the stripes (in pixels) at the point closest to the camera
#define NEAR 16

static CopListT *coplist[2];
// ID of the active copper list [0-1]
static short active = 0;

static CopInsT *copLine[2][HEIGHT];
// Width of the leftmost stripe (in pixels) at any given scanline
static short stripeWidth[HEIGHT];
// Light level (values [0-11] where 11 is the darkest) at any given scanline,
// used to depth to the stripe's colors
static short stripeLight[HEIGHT];

// A struct that controls stripe's colors:
// step - counter indicating when transition to the next color in the cycle
// orig - initial color value from which we start the color cycle
// color- current stripe's color
typedef struct {
  short step, orig, color;
} StripeT;

static StripeT stripe[15];
// A table for storing colors as they are being shifted
static short rotated[15];
static u_char bplShifterValues[4096];

#include "data/stripes.c"
#include "data/floor.c"

// Set the base light level for every given scanline
static void GenerateStripeLight(void) {
  short *light = stripeLight;
  // The higher, the more dimmed screen will be. Going under 11 or over 15 will 
  // cause visual glitches.
  short level = 11; 
  short i;

  // Dim the upper half of the screen (the "wall")
  for (i = 0; i < HEIGHT / 2; i++)
    *light++ = level;
  // Set light gradient for the lower half of the screen (the "floor")
  for (i = 0; i < HEIGHT / 2; i++)
    *light++ = level - (12 * i) / (HEIGHT / 2);
}

// Calculate width of the leftmost stripe at any given scanline
static void GenerateStripeWidth(void) {
  short *width = stripeWidth;
  short i;
  
  for (i = 0; i < HEIGHT / 2; i++)
    // The width is static for the upper half
    *width++ = (FAR << 4);
  for (i = 0; i < HEIGHT / 2; i++)
    *width++ = (FAR << 4) + ((i << 4) * (NEAR - FAR)) / (HEIGHT / 2);
}

// This one is a bit tricky - generate a table of values that will be written to
// BPLCON1 (PF1Px and PF2Px bits) while shifting the playfields. If you are
// wondering why this table is so big and has a lot of repeating values - it
// makes fetching shift values much faster and easier in the ShiftStripes()
// function that is executed each frame.
static void GenerateBPLShifterValues(void) {
  u_char *data = bplShifterValues;
  short i, j;
  
  // 16 - for every possible offset
  for (j = 0; j < 16; j++) {
    // 256 - for each scanline
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
    // Every stripe starts black
    s->color = 0;
    s++;
  }
}

static void Init(void) {
  GenerateBPLShifterValues();
  GenerateStripeLight();
  GenerateStripeWidth();
  InitStripes();

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

// Shift colors by an offset
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

/* Calculate the color of the stripe, taking the light intesivity into consideration. */
static void ColorizeStripes(CopInsT **stripeLine) {
  short i;

  for (i = 1; i < 16; i++) {
    CopInsT **line = stripeLine;
    short *light = stripeLight;
    short n = HEIGHT / 8;
    short r, g, b;

    {
      // Get the offseted color of the stripe
      short s = rotated[i - 1];

      // Extract RGB values from color
      r = s & 0xf00;
      s <<= 4;
      g = s & 0xf00;
      s <<= 4;
      b = s & 0xf00;
    }

    // Each 8 lines make colors one level brighter
    while (--n >= 0) {
      // Set the light value
      u_char *tab = colortab + (*light);
      // Write new RGB values back into one variable
      short color = (tab[r] << 4) | (u_char)(tab[g] | (tab[b] >> 4));

      CopInsSet16(*line + i, color);

      // Increment the pointers
      line += 8; light += 8;
    }
  }
}

static void ShiftStripes(CopInsT **line, short offset) {
  short *width = stripeWidth;
  u_char *data = bplShifterValues;
  u_char *ptr;
  short n = HEIGHT;

  // Offset the starting point 
  offset = (offset & 15) << 8;
  data += offset;
  
  // Get 
  while (--n >= 0) {
    // The underlying instruction is:
    // CopMove16(cp, bplcon1, data[*width++]);
    ptr = (u_char *)(*line++);
    ptr[3] = data[*width++];
  }
}

static void ControlStripes(void) {
  StripeT *s = stripe;
  short diff = frameCount - lastFrameCount;
  short n = 15;

  while (--n >= 0) {
    // Decrement the color counter
    s->step -= diff;
    if (s->step < -128) {
      // If we've reached the end of the cycle, start it again at a random point
      s->step = 64 + (random() & 255);
    }

    if (s->step >= 0) {
      short step = s->step / 8;
      short from, to;

      if (step > 15) {
        // Make color go brighter
        from = s->orig;
        to = 0xfff;
        step -= 16;
      } else {
        // Start going back to the original color
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

EFFECT(floor, NULL, NULL, Init, Kill, Render);
