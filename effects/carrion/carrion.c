#include "effect.h"
#include "copper.h"
#include "pixmap.h"
#include "gfx.h"

#include "data/carrion-metro-pal.c"
#include "data/carrion-metro-data.c"

static CopListT *cp;

static CopListT *MakeCopperList(void) {
  CopListT *cp = NewCopList(100 + carrion_height * (carrion_cols_width + 1));

  CopSetupBitplanes(cp, &carrion, carrion_depth);

  {
    u_short *data = carrion_cols_pixels;
    short i, j;

    for (i = 0; i < carrion_height; i++) {
      for (j = 0; j < carrion_cols_width; j++) {
        CopMove16(cp, color[j], *data++);
      }

      /* Start exchanging palette colors at the end of line. */
      CopWait(cp, Y(i-1), X(carrion_width + 16));
    }
  }

  return CopListFinish(cp);
}

static void Init(void) {
  SetupPlayfield(MODE_LORES, carrion_depth,
                 X(0), Y(0), carrion_width, carrion_height);

  cp = MakeCopperList();
  CopListActivate(cp);
  EnableDMA(DMAF_RASTER);
}

static void Kill(void) {
  DeleteCopList(cp);
}

EFFECT(Carrion, NULL, NULL, Init, Kill, NULL, NULL);
