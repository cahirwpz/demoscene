#include "startup.h"
#include "hardware.h"
#include "coplist.h"
#include "gfx.h"
#include "ilbm.h"

static BitmapT *bitmap;
static CopListT *cp;

static void Load() {
  bitmap = LoadILBMCustom("data/test.ilbm", BM_KEEP_PACKED|BM_LOAD_PALETTE);
  cp = NewCopList(100);
}

static void UnLoad() {
  DeleteCopList(cp);
  DeletePalette(bitmap->palette);
  DeleteBitmap(bitmap);
}

static void Init() {
  {
    LONG lines = ReadLineCounter();
    BitmapUnpack(bitmap, BM_DISPLAYABLE);
    lines = ReadLineCounter() - lines;
    Log("Bitmap unpacking took %ld raster lines.\n", (LONG)lines);
  }

  CopInit(cp);
  CopMakePlayfield(cp, NULL, bitmap, bitmap->depth);
  CopMakeDispWin(cp, X(0), Y(0), bitmap->width, bitmap->height);
  CopLoadPal(cp, bitmap->palette, 0);
  CopEnd(cp);

  CopListActivate(cp);
  custom->dmacon = DMAF_SETCLR | DMAF_RASTER;
}

EffectT Effect = { Load, UnLoad, Init, NULL, NULL };
