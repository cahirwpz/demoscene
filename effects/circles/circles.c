#include "effect.h"
#include "copper.h"
#include "circle.h"

#define WIDTH 320
#define HEIGHT 256
#define DEPTH 1

static BitmapT *screen;
static CopListT *cp;

static void Load(void) {
  screen = NewBitmap(WIDTH, HEIGHT, DEPTH);
  cp = NewCopList(100);

  CopInit(cp);
  CopSetupGfxSimple(cp, MODE_LORES, DEPTH, X(0), Y(0), WIDTH, HEIGHT);
  CopSetupBitplanes(cp, NULL, screen, DEPTH);
  CopSetColor(cp, 0, 0x000);
  CopSetColor(cp, 1, 0xfff);
  CopEnd(cp);
}

static void UnLoad(void) {
  DeleteCopList(cp);
  DeleteBitmap(screen);
}

PROFILE(DrawCircle);

static void Init(void) {
  CopListActivate(cp);
  EnableDMA(DMAF_RASTER);

  ProfilerStart(DrawCircle);
  {
    short r;

    for (r = 2; r < screen->height / 2 - 2; r += 2)
      Circle(screen, 0, screen->width / 2, screen->height / 2, r);
  }
  ProfilerStop(DrawCircle);
}

EFFECT(circles, Load, UnLoad, Init, NULL, NULL);
