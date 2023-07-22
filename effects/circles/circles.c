#include "effect.h"
#include "copper.h"
#include "circle.h"

#define WIDTH 320
#define HEIGHT 256
#define DEPTH 1

static BitmapT *screen;
static CopListT *cp;

static void Init(void) {
  screen = NewBitmap(WIDTH, HEIGHT, DEPTH, BM_CLEAR);

  SetupPlayfield(MODE_LORES, DEPTH, X(0), Y(0), WIDTH, HEIGHT);
  SetColor(0, 0x000);
  SetColor(1, 0xfff);

  cp = NewCopList(100);
  CopSetupBitplanes(cp, screen, DEPTH);
  CopListFinish(cp);

  CopListActivate(cp);
  EnableDMA(DMAF_RASTER);
}

static void Kill(void) {
  DeleteCopList(cp);
  DeleteBitmap(screen);
}

PROFILE(DrawCircle);

static void Render(void) {
  ProfilerStart(DrawCircle);
  {
    short r;

    for (r = 2; r < screen->height / 2 - 2; r += 2)
      Circle(screen, 0, screen->width / 2, screen->height / 2, r);
  }
  ProfilerStop(DrawCircle);
  TaskWaitVBlank();
}

EFFECT(Circles, NULL, NULL, Init, Kill, Render, NULL);
