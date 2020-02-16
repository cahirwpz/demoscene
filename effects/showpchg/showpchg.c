#include "startup.h"
#include "hardware.h"
#include "coplist.h"
#include "gfx.h"
#include "ilbm.h"

#define WIDTH 320
#define HEIGHT 256
#define DEPTH 6

const char *__cwdpath = "data";

static BitmapT *bitmap;
static CopListT *cp;

static void Load(void) {
  bitmap = LoadILBMCustom("face-pchg.ilbm", BM_KEEP_PACKED|BM_LOAD_PALETTE);
}

static void UnLoad(void) {
  DeletePalette(bitmap->palette);
  DeleteBitmap(bitmap);
}

static void Init(void) {
  short w = bitmap->width;
  short h = bitmap->height;
  short xs = X((WIDTH - w) / 2);
  short ys = Y((HEIGHT - h) / 2);

  {
    int lines = ReadLineCounter();
    BitmapUnpack(bitmap, BM_DISPLAYABLE);
    lines = ReadLineCounter() - lines;
    Log("Bitmap unpacking took %d raster lines.\n", lines);
  }

  cp = NewCopList(100 + bitmap->pchgTotal + bitmap->height * 2);

  CopInit(cp);
  CopSetupGfxSimple(cp, MODE_HAM, DEPTH, xs, ys, w, h);
  CopSetupBitplanes(cp, NULL, bitmap, DEPTH);
  CopLoadPal(cp, bitmap->palette, 0);

  if (bitmap->pchg) {
    u_short *data = bitmap->pchg;
    short i;

    for (i = 0; i < bitmap->height; i++) {
      short count = *data++;

      CopWait(cp, Y(i), 0);

      while (count-- > 0) {
        u_short change = *data++;
        CopMove16(cp, color[change >> 12], change & 0xfff);
      }
    }
  }

  CopEnd(cp);

  Log("Used copper list slots: %ld\n", (ptrdiff_t)(cp->curr - cp->entry));

  CopListActivate(cp);
  EnableDMA(DMAF_RASTER);
}

static void Kill(void) {
  DeleteCopList(cp);
}

EffectT Effect = { Load, UnLoad, Init, Kill, NULL, NULL };
