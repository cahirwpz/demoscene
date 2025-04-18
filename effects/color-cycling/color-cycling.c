#include <effect.h>
#include <copper.h>
#include <blitter.h>
#include <gfx.h>
#include <sync.h>
#include <palette.h>

static __code CopListT *cp;

#include "data/image.c"

/*
 * http://amigadev.elowar.com/read/ADCD_2.1/Devices_Manual_guide/node0281.html
 */

typedef struct ColorCycling {
  short rate;
  short step;
  short ncolors;
  short *cells;
  u_short *colors;
} ColorCyclingT;

#include "data/image-cycling.c"

/* I genuinely hate this format. Reverse engineering it is PITA! */
#define PAL_FRAME ((1 << 16) / 50)

static void ColorCyclingStep(ColorCyclingT *rots, short len) {
  do {
    ColorCyclingT *rot = rots++;

    rot->step += PAL_FRAME;

    if (rot->step >= rot->rate) {
      short *cells = rot->cells;
      short reg;

      rot->step -= rot->rate;

      while ((reg = *cells++) >= 0) {
        short idx = *cells + 1; 
        if (idx == rot->ncolors)
          idx = 0;
        *cells++ = idx;

        SetColor(reg, rot->colors[idx]);
      }
    }
  } while (--len > 0);
}

static void Init(void) {
  SetupPlayfield(MODE_LORES, logo_depth, X(0), Y(0), logo_width, logo_height);
  LoadColors(logo_colors, 0);

  cp = NewCopList(40);
  CopSetupBitplanes(cp, &logo, logo_depth);
  CopListFinish(cp);
  CopListActivate(cp);

  EnableDMA(DMAF_RASTER);
}

static void Kill(void) {
  CopperStop();
  DeleteCopList(cp);
}

static void Render(void) {
  ColorCyclingStep(logo_cycling, nitems(logo_cycling));
  TaskWaitVBlank();
}

EFFECT(Logo, NULL, NULL, Init, Kill, Render, NULL);
