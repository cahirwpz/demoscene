#include "effect.h"
#include "copper.h"
#include "gfx.h"

#include "data/cathedral-light.c"
#include "data/cathedral-dark.c"

static __code CopListT *cp;

static CopListT *MakeCopperList(void) {
  CopListT *cp = NewCopList(32);
  CopSetupBitplanes(cp, &cathedral_light, cathedral_light_depth);
  return CopListFinish(cp);
}

static void Init(void) {
  (void)cathedral_dark;

  SetupPlayfield(MODE_LORES, cathedral_light_depth,
                 X(0), Y(0), cathedral_light_width, cathedral_light_height);
  LoadColors(cathedral_colors, 0);

  cp = MakeCopperList();
  CopListActivate(cp);
  EnableDMA(DMAF_RASTER);
}

static void Kill(void) {
  CopperStop();
  DeleteCopList(cp);
}

EFFECT(Cathedral, NULL, NULL, Init, Kill, NULL, NULL);
