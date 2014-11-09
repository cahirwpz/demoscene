#include "startup.h"
#include "coplist.h"
#include "circle.h"

#define WIDTH 320
#define HEIGHT 256
#define DEPTH 1

static BitmapT *screen;
static CopListT *cp;

static void Load() {
  screen = NewBitmap(WIDTH, HEIGHT, DEPTH);
  cp = NewCopList(100);

  CopInit(cp);
  CopMakePlayfield(cp, NULL, screen, DEPTH);
  CopMakeDispWin(cp, X(0), Y(0), screen->width, screen->height);
  CopSetRGB(cp, 0, 0x000);
  CopSetRGB(cp, 1, 0xfff);
  CopEnd(cp);
}

static void UnLoad() {
  DeleteCopList(cp);
  DeleteBitmap(screen);
}

static void Init() {
  CopListActivate(cp);
  custom->dmacon = DMAF_SETCLR | DMAF_RASTER;

  {
    LONG lines = ReadLineCounter();
    WORD r;

    for (r = 2; r < screen->height / 2 - 2; r += 2)
      Circle(screen, 0, screen->width / 2, screen->height / 2, r);

    Log("circles: %ld\n", ReadLineCounter() - lines);
  }
}

EffectT Effect = { Load, UnLoad, Init };
