#include "effect.h"
#include "copper.h"
#include "gfx.h"
#include "blitter.h"
#include "fx.h"

#define WIDTH 256
#define HEIGHT 144
#define DEPTH 4

static BitmapT *foreground;
static CopListT *cp[2];
static const BitmapT *lower;
static u_short *lower_pal;
static Point2D lower_pos;
static Area2D lower_area;
static short active = 0;

/* 'credits_logo' and 'txt_*' must have empty 16 pixels on the left and on the
 * right. Otherwise Display Data Fetcher will show some artifact when image
 * crosses edge of the screen. */

#include "data/01_cahir.c"
#include "data/02_slayer.c"
#include "data/03_jazzcat.c"
#include "data/04_dkl.c"
#include "data/05_dance1.c"
#include "data/06_dance2.c"
#include "data/07_dance3.c"
#include "data/08_dance4.c"
#include "data/credits_logo.c"
#include "data/discoball.c"
#include "data/floor.c"
#include "data/txt_cahir.c"
#include "data/txt_codi.c"
#include "data/txt_dkl.c"
#include "data/txt_jazz.c"
#include "data/txt_slay.c"

static const BitmapT *dance[8] = {
  &cahir, &slayer, &jazzcat, &dkl, &dance1, &dance2, &dance3, &dance4
};

static const BitmapT *member[5] = {
  &txt_cahir, &txt_slay, &txt_jazz, &txt_dkl, &txt_codi
};

#define DISCO_X X((320 - disco.width) / 2)
#define DISCO_Y Y(0)

#define LOGO_Y Y(256 - 64)

#define FLOOR_X X((320 - floor.width) / 2)
#define FLOOR_Y Y(64)

static void MakeCopperList(CopListT *cp) {
  CopListReset(cp);
  CopSetupDisplayWindow(cp, MODE_LORES, X(0), Y(0), 320, 256); 
  CopMove16(cp, dmacon, DMAF_RASTER);

  /* Display disco ball. */
  CopWaitSafe(cp, DISCO_Y - 1, 0);
  CopLoadColors(cp, disco_colors, 0);
  CopSetupMode(cp, MODE_LORES, disco.depth);
  CopSetupBitplanes(cp, &disco, disco.depth);
  CopSetupBitplaneFetch(cp, MODE_LORES, DISCO_X, disco.width);

  CopWaitSafe(cp, DISCO_Y, 0);
  CopMove16(cp, dmacon, DMAF_SETCLR | DMAF_RASTER);
  CopWaitSafe(cp, DISCO_Y + disco.height - 1, LASTHP);
  CopMove16(cp, dmacon, DMAF_RASTER);

  /* Display logo & credits. */
  CopWaitSafe(cp, FLOOR_Y - 1, 0);
  CopLoadColors(cp, floor_colors, 0);
  CopLoadColors(cp, dance_colors, 8);
  CopSetupMode(cp, MODE_DUALPF, 6);
  {
    void *const *planes0 = floor.planes;
    void *const *planes1 = foreground->planes;
    short i;

    for (i = 0; i < 6;) {
      CopMove32(cp, bplpt[i++], *planes0++);
      CopMove32(cp, bplpt[i++], *planes1++);
    }

    CopMove16(cp, bpl1mod, 0);
    CopMove16(cp, bpl2mod, 0);
  }
  CopSetupBitplaneFetch(cp, MODE_LORES, FLOOR_X, floor.width);

  CopWaitSafe(cp, FLOOR_Y, 0);
  CopMove16(cp, dmacon, DMAF_SETCLR | DMAF_RASTER);
  CopWaitSafe(cp, FLOOR_Y + floor.height, LASTHP);
  CopMove16(cp, dmacon, DMAF_RASTER);

  /* Display logo and textual credits. */
  if (lower) {
    /* There're some differences between OCS and ECS that make an artifact
     * visible (on ECS) while 'lower' bitmap is on the left side of the screen.
     * I found 'X(56) / 2' to be the least working horizontal position,
     * but I cannot provide any sound explanation why is it so? */
    CopWaitSafe(cp, LOGO_Y - 1, X(56) / 2);
    CopLoadColorArray(cp, lower_pal, logo_colors_count, 0);
    CopSetupMode(cp, MODE_LORES, lower->depth);
    CopSetupBitplaneArea(cp, MODE_LORES, lower->depth,
                         lower, X(lower_pos.x), Y(lower_pos.y), &lower_area);

    CopWaitSafe(cp, LOGO_Y, 0);
    CopMove16(cp, dmacon, DMAF_SETCLR | DMAF_RASTER);
    CopWaitSafe(cp, LOGO_Y + lower->height - 1, LASTHP);
    CopMove16(cp, dmacon, DMAF_RASTER);
  }

  CopListFinish(cp);
}

static void Init(void) {
  foreground = NewBitmap(max(floor.width, dance[0]->width),
                         max(floor.height, dance[0]->height),
                         floor.depth, BM_CLEAR);

  lower = NULL;

  cp[0] = NewCopList(300);
  cp[1] = NewCopList(300);
  MakeCopperList(cp[0]);
  CopListActivate(cp[0]);
  EnableDMA(DMAF_RASTER | DMAF_BLITTER | DMAF_BLITHOG);
}

static void Kill(void) {
  DisableDMA(DMAF_RASTER | DMAF_BLITTER | DMAF_BLITHOG);
  DeleteCopList(cp[0]);
  DeleteCopList(cp[1]);
  DeleteBitmap(foreground);
}

static void Render(void) {
  if (frameCount > 600) {
    short i = div16(frameCount - 250, 8) & 3;
    lower = &logo;
    lower_pal = logo_colors;
    BitmapCopy(foreground, 80, 0, dance[i + 4]);
  } else if (frameCount > 500) {
    lower = member[4];
    lower_pal = member_colors;
    BitmapCopy(foreground, 80, 0, dance[4]);
  } else if (frameCount > 400) {
    lower = member[3];
    lower_pal = member_colors;
    BitmapCopy(foreground, 80, 0, dance[3]);
  } else if (frameCount > 300) {
    lower = member[2];
    lower_pal = member_colors;
    BitmapCopy(foreground, 80, 0, dance[2]);
  } else if (frameCount > 200) {
    lower = member[1];
    lower_pal = member_colors;
    BitmapCopy(foreground, 80, 0, dance[1]);
  } else if (frameCount > 100) {
    lower = member[0];
    lower_pal = member_colors;
    BitmapCopy(foreground, 80, 0, dance[0]);
  } else {
    lower = NULL;
  }

  if (lower) {
    static const Box2D window = { 0, 0, 319, 255 }; 

    lower_pos.x = ((320 - lower->width) / 2) + (SIN(frameCount * 16) >> 4);
    lower_pos.y = 256 - 64;
    lower_area.x = 0;
    lower_area.y = 0;
    lower_area.w = lower->width;
    lower_area.h = lower->height;

    if (!ClipArea(&window, &lower_pos, &lower_area))
      lower = NULL;
  }

  MakeCopperList(cp[active]);
  CopListRun(cp[active]);
  TaskWaitVBlank();
  active ^= 1;
}

EFFECT(Credits, NULL, NULL, Init, Kill, Render, NULL);
