#include "startup.h"
#include "hardware.h"
#include "coplist.h"
#include "gfx.h"
#include "ilbm.h"

static BitmapT *bitmap;
static CopListT *cp;

static void Load() {
  bitmap = LoadILBM("data/test.ilbm", FALSE);
  cp = NewCopList(100);
}

static void Kill() {
  DeleteCopList(cp);
  DeletePalette(bitmap->palette);
  DeleteBitmap(bitmap);
}

static void Init() {
  CopInit(cp);
  CopMakePlayfield(cp, NULL, bitmap);
  CopMakeDispWin(cp, X(0), Y(0), bitmap->width, bitmap->height);
  CopLoadPal(cp, bitmap->palette, 0);
  CopEnd(cp);

  CopListActivate(cp);
  custom->dmacon = DMAF_SETCLR | DMAF_RASTER;
}

static void Loop() {
  WaitMouse();
}

EffectT Effect = { Load, Kill, Init, Loop };
