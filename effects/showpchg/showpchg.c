#include "effect.h"
#include "hardware.h"
#include "copper.h"
#include "gfx.h"

#define WIDTH 320
#define HEIGHT 256
#define DEPTH 6

#include "data/face.c"

static CopListT *cp;

static void Init(void) {
  short w = face.width;
  short h = face.height;
  short xs = X((WIDTH - w) / 2);
  short ys = Y((HEIGHT - h) / 2);

  cp = NewCopList(100 + face_pchg_count + face.height * 2);

  CopInit(cp);
  CopSetupGfxSimple(cp, MODE_HAM, DEPTH, xs, ys, w, h);
  CopSetupBitplanes(cp, NULL, &face, DEPTH);

  {
    u_short *data = face_pchg;
    short i;

    for (i = 0; i < face.height; i++) {
      short count = *data++;

      while (count-- > 0) {
        u_short change = *data++;
        CopMove16(cp, color[change & 15], change >> 4);
      }

      /* Start exchanging palette colors at the end of line. */
      CopWait(cp, Y(i), X(face.width + 16));
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

EFFECT(showpchg, NULL, NULL, Init, Kill, NULL);
