#include <effect.h>
#include <2d.h>
#include <blitter.h>
#include <copper.h>
#include <fx.h>
#include <system/memory.h>

#define WIDTH  320
#define HEIGHT 240
#define DEPTH  4

static BitmapT *screen;
static CopInsPairT *bplptr;
static CopListT *cp;
static short active = 0;

typedef struct {
  short width, height;
  short current, count;
  u_char *frame[0];
} AnimSpanT;

#include "data/running-pal.c"
#include "data/running.c"

static void Load(void) {
  screen = NewBitmap(WIDTH, HEIGHT, DEPTH + 1, BM_CLEAR);

  Log("Animation has %d frames %d x %d.\n", 
      running.count, running.width, running.height);
}

static void UnLoad(void) {
  DeleteBitmap(screen);
}

static void Init(void) {
  EnableDMA(DMAF_BLITTER);
  BitmapClear(screen);

  SetupPlayfield(MODE_LORES, DEPTH, X(0), Y(0), WIDTH, HEIGHT);
  LoadPalette(&running_pal, 0);

  cp = NewCopList(100);
  CopInit(cp);
  bplptr = CopSetupBitplanes(cp, screen, DEPTH);
  CopEnd(cp);

  CopListActivate(cp);
  EnableDMA(DMAF_RASTER);
}

static void Kill(void) {
  DisableDMA(DMAF_COPPER | DMAF_RASTER | DMAF_BLITTER);

  DeleteCopList(cp);
}

static void DrawSpans(u_char *bpl) {
  u_char *frame = running.frame[running.current];
  short f = normfx(SIN(frameCount * 32) * 48);
  short n = running.height;
  short stride = screen->bytesPerRow;

  WaitBlitter();

  while (--n >= 0) {
    short m = *frame++;

    while (--m >= 0) {
      short x = *frame++;
      x += f;
      bset(bpl + (x >> 3), ~x);
    }

    bpl += stride;
  }

  running.current++;

  if (running.current >= running.count)
    running.current -= running.count;
}

PROFILE(AnimRender);

static void Render(void) {
  ProfilerStart(AnimRender);
  {
    BlitterClear(screen, active);
    DrawSpans(screen->planes[active]);
    BlitterFill(screen, active);
  }
  ProfilerStop(AnimRender);

  {
    short n = DEPTH;

    while (--n >= 0) {
      short i = (active + n + 1 - DEPTH) % (DEPTH + 1);
      if (i < 0)
        i += DEPTH + 1;
      CopInsSet32(&bplptr[n], screen->planes[i]);
    }
  }

  TaskWaitVBlank();

  active = (active + 1) % (DEPTH + 1);
}

EFFECT(Anim, Load, UnLoad, Init, Kill, Render, NULL);
