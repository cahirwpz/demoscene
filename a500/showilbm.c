#include "hardware.h"
#include "coplist.h"
#include "gfx.h"
#include "ilbm.h"

static BitmapT *bitmap;

void Load() {
  bitmap = LoadILBM("test.ilbm", FALSE);
}

static inline void
MakeDisplayWindow(CopListT *cp, UBYTE xs, UBYTE ys, UWORD w, UWORD h) {
  /* vstart  $00 ..  $ff */
  /* hstart  $00 ..  $ff */
  /* vstop   $80 .. $17f */
  /* hstop  $100 .. $1ff */
  UBYTE xe = xs + w;
  UBYTE ye = ys + h;

  CopMove16(cp, diwstrt, (ys << 8) | xs);
  CopMove16(cp, diwstop, (ye << 8) | xe);
}

#define BPLCON0_BPU(d)  (((d) & 7) << 12)
#define BPLCON0_COLOR   (1 << 9)

void Main() {
  CopListT *cp = NewCopList(100);
  WORD i;

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

  MakeDisplayWindow(cp, 0x81, 0x2c, bitmap->width, bitmap->height);

  for (i = 0; i < bitmap->depth; i++)
    CopMove32(cp, bplpt[i], (ULONG)bitmap->planes[i]);

  for (i = 0; i < bitmap->palette->count; i++) {
    ColorT c = bitmap->palette->colors[i];

    CopMove16(cp, color[i],
              ((c.r & 0xf0) << 4) | (c.g & 0xf0) | ((c.b & 0xf0) >> 4));
  }

  CopEnd(cp);
  CopListActivate(cp);

  custom->dmacon = DMAF_SETCLR | DMAF_RASTER | DMAF_MASTER;

  WaitMouse();
  DeleteCopList(cp);
  DeletePalette(bitmap->palette);
  DeleteBitmap(bitmap);
}
