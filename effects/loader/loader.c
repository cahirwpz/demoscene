#include <effect.h>
#include <copper.h>
#include <blitter.h>
#include <gfx.h>
#include <line.h>
#include <ptplayer.h>
#include <color.h>

#define _SYSTEM
#include <system/cia.h>

#include "data/loader.c"

extern u_char LoaderModule[];
extern u_char LoaderSamples[];

static __code BitmapT *screen;
static __code CopListT *cp;

#define X1 99
#define Y1 230
#define X2 220
#define Y2 238

#define WIDTH 320
#define HEIGHT 256
#define DEPTH 3

static void Init(void) {
  PtInstallCIA();
  PtInit(LoaderModule, LoaderSamples, 1);
  PtEnable = 1;

  screen = NewBitmap(WIDTH, HEIGHT, DEPTH, BM_CLEAR);
  cp = NewCopList(40);

  CpuLineSetup(screen, 0);
  CpuLine(X1 - 1, Y1 - 2, X2 + 1, Y1 - 2);
  CpuLine(X1 - 1, Y2 + 1, X2 + 1, Y2 + 1);
  CpuLine(X1 - 2, Y1 - 1, X1 - 2, Y2 + 1);
  CpuLine(X2 + 2, Y1 - 1, X2 + 2, Y2 + 1);

  SetupPlayfield(MODE_LORES, DEPTH, X(0), Y(0), WIDTH, HEIGHT);
  LoadColors(loader_colors, 0);

  EnableDMA(DMAF_BLITTER);
  BitmapCopy(screen, 0, 0, &loader);
  WaitBlitter();
  DisableDMA(DMAF_BLITTER);

  CopSetupBitplanes(cp, screen, DEPTH);
  CopListFinish(cp);
  CopListActivate(cp);

  EnableDMA(DMAF_RASTER);
}

static void Kill(void) {
  BlitterStop();
  CopperStop();

  DeleteCopList(cp);
  DeleteBitmap(screen);

  PtEnd();
  PtRemoveCIA();
  DisableDMA(DMAF_AUDIO);
}

static void Render(void) {
  static __code short x = 0;
  short newX = frameCount >> 3;
  if (newX > 121)
    newX = 122;
  for (; x < newX; x++) {
    CpuLine(X1 + x, Y1, X1 + x, Y2);
  }
}

EFFECT(Loader, NULL, NULL, Init, Kill, Render, NULL);
