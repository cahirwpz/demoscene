#include "effect.h"
#include "custom.h"
#include "copper.h"
#include "bitmap.h"
#include "palette.h"
#include "profiler.h"

#include "data/stars-die.c"

#define LINES 256
#define DEPTH 5

static CopListT *cp[2];
static CopInsT *bplptr[2][DEPTH];
static short active = 0;

/* static */ void VerticalScalerForward(CopListT *cp, short ys, short height) {
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
/* static */ void MakeCopperList(CopListT *cp, CopInsT **bplptr, short height) {
  short ys = (LINES - height) / 2;

  CopInit(cp);
  CopSetupDisplayWindow(cp, MODE_LORES, X(0), Y(ys), image.width, height);
  CopSetupBitplanes(cp, bplptr, &image, image.depth);
  VerticalScalerForward(cp, ys, height);
  CopEnd(cp);
}

/* static */ void Init(void) {
  LoadPalette(&image_pal, 0);
  SetupPlayfield(MODE_LORES, image.depth,
                 X(0), Y(0), image.width, image.height);

  cp[0] = NewCopList(40 + LINES * 3);
  cp[1] = NewCopList(40 + LINES * 3);
  MakeCopperList(cp[active], bplptr[active], LINES);
  CopListActivate(cp[active]);

  EnableDMA(DMAF_RASTER);
}

static void Kill(void) {
  DisableDMA(DMAF_COPPER | DMAF_RASTER);
  DeleteCopList(cp[0]);
  DeleteCopList(cp[1]);
}

PROFILE(Scaler);

static void Render(void) {
  static short val = 0, dir = 1;

  ProfilerStart(Scaler);
  MakeCopperList(cp[active], bplptr[active], LINES - val);
  ProfilerStop(Scaler);

  CopListRun(cp[active]);
  TaskWaitVBlank();

  val += dir;
  if ((val == 0) || (val == LINES - 2))
    dir = -dir;

  active ^= 1;
}

EFFECT(scaler, NULL, NULL, Init, Kill, Render);
