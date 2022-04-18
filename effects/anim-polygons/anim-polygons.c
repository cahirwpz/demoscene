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
#define DEPTH 4

#define HEIGHT_P1 210

static BitmapT *screen;
static CopInsT *bplptr[DEPTH];
static CopListT *cp;
static short active = 0;

#include "anim_data.c"
#include "anim-pal.c"

/* Reading polygon data */
static short current_frame = 0;
static short frame_count = 260 - 5;
static short *verts_ptr = verts;
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

    for (i = 0; i < HEIGHT_P1 / 10; i++) {
      CopWait(cp, Y(i * 10 - 1), 0xde);
      for (j = 0; j < 16; j++) CopSetColor(cp, j, *pixels++);
    }
  }
  CopEnd(cp);
}

static void Init(void) {
  EnableDMA(DMAF_BLITTER);
  BitmapClear(screen);

  SetupPlayfield(MODE_LORES, DEPTH, X(0), Y(0), WIDTH, HEIGHT);
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
  /* (x/y) previous */
  short xp = 0, yp = 0;
  /* (x/y) end */
  short xe = 0, ye = 0;
  /* (x/y) first in line strip */
  short xf = 0, yf = 0;
  /* -- */
  short *vs = verts_ptr;
  short n = 0;
  u_short xy = 0;
  bool shape_start = true;

  WaitBlitter();

  n = verts_in_frame[current_frame];
  while (--n) {
    if (shape_start) {
      /* first vert in line strip */
      xy = *vs++;
      CalculateXY(xy, xp, yp);
      xf = xp;
      yf = yp;
      shape_start = false;
    }
    xy = *vs++;
    if (xy == 65535) {
      /* no more verts in line strip */
      BlitterLine(xp, yp, xf, yf);
      shape_start = true;
    } else {
      CalculateXY(xy, xe, ye);
      BlitterLine(xp, yp, xe, ye);
      xp = xe;
      yp = ye;
    }
  }

  current_frame++;
  verts_ptr = vs;

  if (current_frame > frame_count) {
    current_frame = 0;
    verts_ptr = verts;
  }
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

EFFECT(anim - lines, Load, UnLoad, Init, Kill, Render);
