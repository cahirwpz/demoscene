#include <demo.h>
#include <debug.h>
#include <copper.h>
#include <blitter.h>
#include <gfx.h>
#include <line.h>

#define _SYSTEM
#include <system/memory.h>
#include <system/interrupt.h>

#include "data/loader.c"

#define X1 96
#define Y1 (120 + 32)
#define X2 224
#define Y2 (136 + 24)

/* from 0 to 256 */
static volatile short LoadProgress = 0;

static int ProgressBarUpdate(void) {
  static __code short x = 0;
  short newX = LoadProgress >> 1;
  for (; x < newX; x++) {
    CpuLine(X1 + x, Y1, X1 + x, Y2);
  }
  return 0;
}

INTSERVER(ProgressBarInterrupt, 0, (IntFuncT)ProgressBarUpdate, NULL);

static void LoadData(void) {
  while (LoadProgress < 256) {
    LoadProgress++;
    WaitVBlank();
  }
}

#define WIDTH 320
#define HEIGHT 256
#define DEPTH 1

void LoadDemo(void) {
  BitmapT *screen = NewBitmap(WIDTH, HEIGHT, DEPTH, BM_CLEAR);
  CopListT *cp = NewCopList(40);

  CpuLineSetup(screen, 0);
  CpuLine(X1 - 1, Y1 - 2, X2 + 1, Y1 - 2);
  CpuLine(X1 - 1, Y2 + 1, X2 + 1, Y2 + 1);
  CpuLine(X1 - 2, Y1 - 1, X1 - 2, Y2 + 1);
  CpuLine(X2 + 2, Y1 - 1, X2 + 2, Y2 + 1);

  SetupPlayfield(MODE_LORES, DEPTH, X(0), Y(0), WIDTH, HEIGHT);
  LoadColors(loader_colors, 0);

  EnableDMA(DMAF_BLITTER);
  BitmapCopy(screen, (WIDTH - loader_width) / 2, Y1 - loader_height - 16,
             &loader);
  WaitBlitter();
  DisableDMA(DMAF_BLITTER);

  CopSetupBitplanes(cp, screen, DEPTH);
  CopListFinish(cp);
  CopListActivate(cp);

  EnableDMA(DMAF_RASTER);

  AddIntServer(INTB_VERTB, ProgressBarInterrupt);
  LoadData();
  RemIntServer(INTB_VERTB, ProgressBarInterrupt);

  DisableDMA(DMAF_RASTER);
  DeleteCopList(cp);

  DeleteBitmap(screen);
}
