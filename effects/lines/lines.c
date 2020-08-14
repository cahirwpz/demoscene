#include "effect.h"
#include "blitter.h"
#include "copper.h"
#include "line.h"

#define WIDTH 320
#define HEIGHT 256
#define DEPTH 1

#define CPULINE 0

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

PROFILE(Lines);

static void Init(void) {
  CopListActivate(cp);
  EnableDMA(DMAF_BLITTER | DMAF_RASTER | DMAF_BLITHOG);

  ProfilerStart(Lines);
  {
    short i;

#if CPULINE == 1
    CpuLineSetup(screen, 0);
#else
    BlitterLineSetup(screen, 0, LINE_OR|LINE_SOLID);
#endif

    for (i = 0; i < screen->width; i += 2) {
#if CPULINE == 1
      CpuLine(i, 0, screen->width - 1 - i, screen->height - 1);
#else
      BlitterLine(i, 0, screen->width - 1 - i, screen->height - 1);
#endif
    }

    for (i = 0; i < screen->height; i += 2) {
#if CPULINE == 1
      CpuLine(0, i, screen->width - 1, screen->height - 1 - i);
#else
      BlitterLine(0, i, screen->width - 1, screen->height - 1 - i);
#endif
    }
  }
  ProfilerStop(Lines);
}

EFFECT(lines, Load, UnLoad, Init, NULL, NULL);
