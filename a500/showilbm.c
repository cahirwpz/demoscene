#include "hardware.h"
#include "coplist.h"
#include "gfx.h"
#include "ilbm.h"

static BitmapT *bitmap;

void Load() {
  bitmap = LoadILBM("test.ilbm");
}

static inline void MakeDisplayWindow(UBYTE xs, UBYTE ys, UWORD w, UWORD h) {
  /* vstart  $00 ..  $ff */
  /* hstart  $00 ..  $ff */
  /* vstop   $80 .. $17f */
  /* hstop  $100 .. $1ff */
  UBYTE xe = xs + w;
  UBYTE ye = ys + h;

  custom->diwstrt = (ys << 8) | xs;
  custom->diwstop = (ye << 8) | xe;
}

void Main() {
  WORD i;

  for (i = 0; i < bitmap->palette->count; i++) {
    ColorT c = bitmap->palette->colors[i];

    custom->color[i] =
      ((c.r & 0xf0) << 4) | (c.g & 0xf0) | ((c.b & 0xf0) >> 4);
  }

  custom->bplcon0 = ((bitmap->depth & 7) << 12) | (1 << 9);
  custom->bplcon1 = 0;
  custom->bplcon2 = 0;
  custom->bpl1mod = 0;
  custom->bpl2mod = 0;
  custom->ddfstrt = 0x38;
  custom->ddfstop = 0xd0;

  MakeDisplayWindow(0x81, 0x2c, bitmap->width, bitmap->height);

  {
    CopListT *cp = NewCopList(100);

    CopInit(cp);

    for (i = 0; i < bitmap->depth; i++)
      CopMove32(cp, CSREG(bplpt[i]), (ULONG)bitmap->planes[i]);

    CopEnd(cp);
    CopListActivate(cp);

    custom->dmacon = DMAF_SETCLR | DMAF_RASTER | DMAF_MASTER;

    WaitMouse();
    DeleteCopList(cp);
  }

  DeletePalette(bitmap->palette);
  DeleteBitmap(bitmap);
}
