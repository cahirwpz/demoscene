#include "startup.h"
#include "hardware.h"
#include "coplist.h"
#include "gfx.h"
#include "ilbm.h"

#define DEPTH 6

static BitmapT *bitmap;
static CopListT *cp;

static void Load() {
  bitmap = LoadILBMCustom("data/face-pchg.ilbm", BM_KEEP_PACKED|BM_LOAD_PALETTE);
}

static void UnLoad() {
  DeletePalette(bitmap->palette);
  DeleteBitmap(bitmap);
}

static void Init() {
  WORD w = bitmap->width;
  WORD h = bitmap->height;
  WORD xs = X((320 - w) / 2);
  WORD ys = Y((256 - h) / 2);

  {
    LONG lines = ReadLineCounter();
    BitmapUnpack(bitmap, BM_DISPLAYABLE);
    lines = ReadLineCounter() - lines;
    Log("Bitmap unpacking took %ld raster lines.\n", (LONG)lines);
  }

  cp = NewCopList(100 + bitmap->pchgTotal + bitmap->height * 2);

  CopInit(cp);
  CopSetupGfxSimple(cp, MODE_HAM, DEPTH, xs, ys, w, h);
  CopSetupBitplanes(cp, NULL, bitmap, DEPTH);
  CopLoadPal(cp, bitmap->palette, 0);

  if (bitmap->pchg) {
    UWORD *data = bitmap->pchg;
    WORD i;

    for (i = 0; i < bitmap->height; i++) {
      WORD count = *data++;

      CopWait(cp, Y(i), 0);

      while (count-- > 0) {
        UWORD change = *data++;
        CopMove16(cp, color[change >> 12], change & 0xfff);
      }
    }
  }

  CopEnd(cp);

  Log("Used copper list slots: %ld\n", (LONG)(cp->curr - cp->entry));

  CopListActivate(cp);
  EnableDMA(DMAF_RASTER);
}

static void Kill() {
  DeleteCopList(cp);
}

EffectT Effect = { Load, UnLoad, Init, Kill, NULL };
