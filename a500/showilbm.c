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

void Main() {
  UWORD i;

  CopInit(cp);
  CopMove16(cp, bplcon0, BPLCON0_BPU(bitmap->depth) | BPLCON0_COLOR);
  CopMove16(cp, bplcon1, 0);
  CopMove16(cp, bplcon2, 0);
  
  {
    UWORD modulo = 0;

    if (bitmap->interleaved)
      modulo = bitmap->width / 8 * (bitmap->depth - 1);

    CopMove16(cp, bpl1mod, modulo);
    CopMove16(cp, bpl2mod, modulo);
  }

  CopMove16(cp, ddfstrt, 0x38);
  CopMove16(cp, ddfstop, 0xd0);

  CopMakeDispWin(cp, 0x81, 0x2c, bitmap->width, bitmap->height);

  for (i = 0; i < bitmap->depth; i++)
    CopMove32(cp, bplpt[i], (ULONG)bitmap->planes[i]);

  CopLoadPal(cp, bitmap->palette);
  CopEnd(cp);
  CopListActivate(cp);

  custom->dmacon = DMAF_SETCLR | DMAF_RASTER | DMAF_MASTER;

  WaitMouse();
}
