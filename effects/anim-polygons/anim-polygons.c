#include "2d.h"
#include "blitter.h"
#include "cia.h"
#include "copper.h"
#include "effect.h"
#include "fx.h"
#include "line.h"
#include "memory.h"
#include "pixmap.h"
#include "types.h"

#define WIDTH 320
#define HEIGHT 180
#define YOFF ((256 - HEIGHT) / 2)
#define DEPTH 4

static BitmapT *screen;
static CopInsT *bplptr[DEPTH];
static CopListT *cp;
static short active = 0;

#include "data/dancing.c"
#include "data/dancing-pal.c"

/* Reading polygon data */
static short current_frame = 0;
/* Synchronization */
static int frame_diff = 0;
static int frame_sync;

static void Load(void) {
  screen = NewBitmap(WIDTH, HEIGHT, DEPTH + 1);
  frame_sync = ReadFrameCounter();
}

static void UnLoad(void) { DeleteBitmap(screen); }

static void MakeCopperList(CopListT *cp) {
  CopInit(cp);
  CopSetupBitplanes(cp, bplptr, screen, DEPTH);
  {
    short *pixels = gradient.pixels;
    short i, j;

    for (i = 0; i < HEIGHT / 10; i++) {
      CopWait(cp, Y(YOFF + i * 10 - 1), 0xde);
      for (j = 0; j < 16; j++) CopSetColor(cp, j, *pixels++);
    }
  }
  CopEnd(cp);
}

static void Init(void) {
  EnableDMA(DMAF_BLITTER);
  BitmapClear(screen);

  SetupPlayfield(MODE_LORES, DEPTH, X(0), Y(YOFF), WIDTH, HEIGHT);
  cp = NewCopList(100 + gradient.height * (gradient.width + 1));
  MakeCopperList(cp);
  CopListActivate(cp);
  EnableDMA(DMAF_RASTER);
}

static void Kill(void) {
  DisableDMA(DMAF_COPPER | DMAF_RASTER | DMAF_BLITTER);
  DeleteCopList(cp);
}

/* Get (x,y) on screen position from linear memory repr */
#define CalculateXY(xy, x, y)           \
  asm("divs %3,%0\n\t"                  \
      "movew %0,%1\n\t"                 \
      "swap %0"                         \
      : "=d" (x), "=r" (y)              \
      : "0" ((int)xy), "dmi" (WIDTH));

static void DrawFrame(void) {
  short *data = dancing_frame[current_frame];
  short n;

  WaitBlitter();

  while ((n = *data++)) {
    /* (x/y) previous */
    short xp, yp;
    /* (x/y) end */
    short xe, ye;
    /* (x/y) first in line strip */
    short xf, yf;

    /* first vert in line strip */
    CalculateXY(*data++, xp, yp);
    xf = xp, yf = yp;

    while (--n > 0) {
      CalculateXY(*data++, xe, ye);
      BlitterLine(xp, yp, xe, ye);
      xp = xe, yp = ye;
    }

    /* last vert in line strip */
    BlitterLine(xp, yp, xf, yf);
  }

  if (++current_frame > dancing_frames - 5)
    current_frame = 0;
}

PROFILE(AnimRender);

static void Render(void) {
  ProfilerStart(AnimRender);
  {
    BlitterClear(screen, active);
    BlitterLineSetup(screen, active, LINE_EOR | LINE_ONEDOT);
    DrawFrame();
    BlitterFill(screen, active);
  }
  ProfilerStop(AnimRender);

  {
    short n = DEPTH;

    while (--n >= 0) {
      short i = mod16(active + n + 1 - DEPTH, DEPTH + 1);
      if (i < 0) i += DEPTH + 1;
      CopInsSet32(bplptr[n], screen->planes[i]);
    }
  }

  /* synchronizing to frame counter */
  frame_diff = ReadFrameCounter() - frame_sync;
  if (frame_diff < 1) {
    TaskWaitVBlank();
  }
  TaskWaitVBlank();

  active = mod16(active + 1, DEPTH + 1);
  frame_sync = ReadFrameCounter();
}

EFFECT(anim_polygons, Load, UnLoad, Init, Kill, Render);
