#include "hardware.h"
#include "coplist.h"
#include "gfx.h"
#include "ilbm.h"

static BitmapT *bitmap;
static CopListT *cp;

void Load() {
  bitmap = LoadILBM("data/test.ilbm", FALSE);
  cp = NewCopList(100);
}

void Kill() {
  DeleteCopList(cp);
  DeletePalette(bitmap->palette);
  DeleteBitmap(bitmap);
}

void Init() {
  CopInit(cp);
  CopMakePlayfield(cp, NULL, bitmap);
  CopMakeDispWin(cp, 0x81, 0x2c, bitmap->width, bitmap->height);
  CopLoadPal(cp, bitmap->palette, 0);
  CopEnd(cp);

  CopListActivate(cp);
  custom->dmacon = DMAF_SETCLR | DMAF_RASTER;
}

void Main() {
  WaitMouse();
}
