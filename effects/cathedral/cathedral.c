#include "bitmap.h"
#include "custom.h"
#include "effect.h"
#include "copper.h"
#include "blitter.h"
#include "gfx.h"

#include "data/cathedral-light.c"
#include "data/cathedral-dark.c"
#include "data/transition.c"

/*
 * bpl[0] -> A0
 * bpl[1] -> B0
 * bpl[2] -> C0
 */
#define EQ_0 NANBNC
#define EQ_1 ANBNC
#define EQ_2 NABNC
#define EQ_3 ABNC
#define EQ_4 NANBC
#define EQ_5 ANBC
#define EQ_6 NABC
#define EQ_7 ABC

#define LE_0 NANBNC
#define LE_1 (ANBNC | LE_0)
#define LE_2 (NABNC | LE_1)
#define LE_3 (ABNC | LE_2)
#define LE_4 (NANBC | LE_3)
#define LE_5 (ANBC | LE_4)
#define LE_6 (NABC | LE_5)
#define LE_7 (ABC | LE_6)

/*
 * bpl[0] -> A0
 * bpl[1] -> B0
 * bpl[2] -> C0
 * bpl[3] -> B1
 * bpl[4] -> C1
 */

static __code const u_short phase_bltcon0[32][2] = {
  {LE_0, EQ_1},
  {LE_1, EQ_1},
  {LE_2, EQ_1},
  {LE_3, EQ_1},
  {LE_4, EQ_1},
  {LE_5, EQ_1},
  {LE_6, EQ_1},
  {LE_7, EQ_1},

  {LE_0, EQ_3|LE_1},
  {LE_1, EQ_3|LE_1},
  {LE_2, EQ_3|LE_1},
  {LE_3, EQ_3|LE_1},
  {LE_4, EQ_3|LE_1},
  {LE_5, EQ_3|LE_1},
  {LE_6, EQ_3|LE_1},
  {LE_7, EQ_3|LE_1},

  {LE_0, EQ_5|LE_3},
  {LE_1, EQ_5|LE_3},
  {LE_2, EQ_5|LE_3},
  {LE_3, EQ_5|LE_3},
  {LE_4, EQ_5|LE_3},
  {LE_5, EQ_5|LE_3},
  {LE_6, EQ_5|LE_3},
  {LE_7, EQ_5|LE_3},

  {LE_0, EQ_7|LE_5},
  {LE_1, EQ_7|LE_5},
  {LE_2, EQ_7|LE_5},
  {LE_3, EQ_7|LE_5},
  {LE_4, EQ_7|LE_5},
  {LE_5, EQ_7|LE_5},
  {LE_6, EQ_7|LE_5},
  {LE_7, EQ_7|LE_5},
};

static __code CopListT *cp;
static __code BitmapT *screen;
static __code BitmapT *mask;

static CopListT *MakeCopperList(void) {
  CopListT *cp = NewCopList(32);
  CopSetupBitplanes(cp, screen, cathedral_light_depth);
  return CopListFinish(cp);
}

static void Init(void) {
  screen = NewBitmap(cathedral_light_width, cathedral_light_height, cathedral_light_depth, 0);
  mask = NewBitmap(cathedral_light_width, cathedral_light_height, 1, 0);

  SetupPlayfield(MODE_LORES, cathedral_light_depth,
                 X(0), Y(0), cathedral_light_width, cathedral_light_height);
  LoadColors(cathedral_colors, 0);

  cp = MakeCopperList();
  CopListActivate(cp);
  EnableDMA(DMAF_RASTER | DMAF_BLITTER | DMAF_BLITHOG);
}

static void Kill(void) {
  DisableDMA(DMAF_BLITTER | DMAF_BLITHOG);
  CopperStop();
  DeleteCopList(cp);
  DeleteBitmap(mask);
  DeleteBitmap(screen);
}

static void CalculateMask(BitmapT *mask, const BitmapT *transition, short phase) {
  custom->bltapt = transition->planes[0];
  custom->bltbpt = transition->planes[1];
  custom->bltcpt = transition->planes[2];
  custom->bltdpt = mask->planes[0];

  custom->bltcon0 = (SRCA | SRCB | SRCC | DEST) | phase_bltcon0[phase][0];
  custom->bltcon1 = 0;
  custom->bltsize = (cathedral_light_height << 6) | (cathedral_light_width >> 4);

  WaitBlitter();

  custom->bltapt = mask->planes[0];
  custom->bltbpt = transition->planes[3];
  custom->bltcpt = transition->planes[4];
  custom->bltdpt = mask->planes[0];

  custom->bltcon0 = (SRCA | SRCB | SRCC | DEST) | phase_bltcon0[phase][1];
  custom->bltcon1 = 0;
  custom->bltsize = (cathedral_light_height << 6) | (cathedral_light_width >> 4);

  WaitBlitter();
}

static void BlitterCopySelect(BitmapT *dst, const BitmapT *src1, const BitmapT *src2, BitmapT *mask) {
  short i;

  for (i = 0; i < dst->depth; i++) {
    custom->bltapt = src1->planes[i];
    custom->bltbpt = src2->planes[i];
    custom->bltcpt = mask->planes[0];
    custom->bltdpt = dst->planes[i];

    custom->bltcon0 = (SRCA | SRCB | SRCC | DEST) | (ABNC | ANBNC | ABC | NABC);
    custom->bltcon1 = 0;
    custom->bltsize = (cathedral_light_height << 6) | (cathedral_light_width >> 4);

    WaitBlitter();
  }
}

PROFILE(Draw);

static void Render(void) {
  const BitmapT *src, *dst;
  short f = frameCount & 255;

  if (f < 128) {
    src = &cathedral_light;
    dst = &cathedral_dark;
  } else {
    src = &cathedral_dark;
    dst = &cathedral_light;
    f -= 128;
  }

  ProfilerStart(Draw);
  {
    CalculateMask(mask, &transition, f >> 2);
    BlitterCopySelect(screen, src, dst, mask);
  }
  ProfilerStop(Draw);

  TaskWaitVBlank();
}

EFFECT(Cathedral, NULL, NULL, Init, Kill, Render, NULL);
