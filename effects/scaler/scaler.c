#include "effect.h"
#include "custom.h"
#include "copper.h"
#include "bitmap.h"
#include "palette.h"
#include "profiler.h"

#include "data/stars-die.c"

#define LINES 256

static CopListT *cp0, *cp1;

/* static */ void VerticalScaler(CopListT *cp, short ys, short height) {
  short rowmod = image.bytesPerRow * image.depth;
  int dy = (LINES << 16) / height;
  short n = (short)(dy >> 16) * (short)image.depth;
  short mod = (short)image.bytesPerRow * (short)(n - 1);
  int y = 0;
  short i;

  for (i = 1; i < height; i++) {
    short _mod = mod;
    int ny = y + dy;
    if ((u_short)ny < (u_short)y)
      _mod += rowmod;
    CopWaitSafe(cp, Y(ys + i), X(0));
    CopMove16(cp, bpl1mod, _mod);
    CopMove16(cp, bpl2mod, _mod);
    y = ny;
  }
}

/* static */ void MakeCopperList(CopListT *cp, short height) {
  short ys = (LINES - height) / 2;

  CopInit(cp);
  CopSetupDisplayWindow(cp, MODE_LORES, X(0), Y(ys), image.width, height);
  CopSetupBitplanes(cp, NULL, &image, image.depth);
  VerticalScaler(cp, ys, height);
  CopEnd(cp);
}

/* static */ void Init(void) {
  LoadPalette(&image_pal, 0);
  SetupPlayfield(MODE_LORES, image.depth,
                 X(0), Y(0), image.width, image.height);

  cp0 = NewCopList(40 + LINES * 3);
  cp1 = NewCopList(40 + LINES * 3);
  MakeCopperList(cp0, LINES);
  CopListActivate(cp0);

  EnableDMA(DMAF_RASTER);
}

static void Kill(void) {
  DisableDMA(DMAF_COPPER | DMAF_RASTER);
  DeleteCopList(cp0);
  DeleteCopList(cp1);
}

PROFILE(Scaler);

static void Render(void) {
  static short val = 0, dir = 1;

  ProfilerStart(Scaler);
  MakeCopperList(cp0, LINES - val);
  ProfilerStop(Scaler);

  CopListRun(cp0);
  TaskWaitVBlank();
  { CopListT *tmp = cp0; cp0 = cp1; cp1 = tmp; }

  val += dir;
  if ((val == 0) || (val == LINES - 2))
    dir = -dir;
}

EFFECT(scaler, NULL, NULL, Init, Kill, Render);
