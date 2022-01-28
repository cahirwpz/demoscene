#include "effect.h"
#include "copper.h"
#include "gfx.h"
#include "pixmap.h"
#include "fx.h"

#define WIDTH 320
#define HEIGHT 256
#define DEPTH 6

#include "data/background.c"
#include "data/foreground.c"

static CopListT *cp0, *cp1;
static short fg_y, bg_y, fg_x, bg_x;

static void MakeCopperList(CopListT *cp) {
  short bg_bplmod = (background.width - (WIDTH + 16)) / 8;
  short fg_bplmod = (foreground.width - (WIDTH + 16)) / 8;

  CopInit(cp);
  CopSetupDisplayWindow(cp, MODE_LORES, X(0), Y(0), WIDTH, HEIGHT);
  CopSetupBitplaneFetch(cp, MODE_LORES, X(-16), WIDTH + 16);
  CopSetupMode(cp, MODE_DUALPF, DEPTH);
  CopLoadPal(cp, &background_pal, 0);
  CopLoadPal(cp, &foreground_pal, 8);
  CopSetColor(cp, 0, 0);
  /* Reverse playfields priorities (for testing) */
#if 0
  CopMove16(cp, bplcon2, 0);
#endif

  /* Setup BPLPT for even bitplanes and BPL1MOD (background) */
  {
    short i;

    for (i = 0; i < background.depth; i++) {
      int offset = mul16(bg_y, background.bytesPerRow) + (bg_x & -16) / 8;
      CopMove32(cp, bplpt[i * 2], background.planes[i] + offset);
    }
    CopMove16(cp, bpl1mod, bg_bplmod);
  }

  /* Setup BPLPT for odd bitplanes and BPL2MOD (background) */
  {
    short i;

    for (i = 0; i < foreground.depth; i++) {
      int offset = mul16(fg_y, foreground.bytesPerRow) + (fg_x & -16) / 8;
      CopMove32(cp, bplpt[i * 2 + 1], foreground.planes[i] + offset);
    }
    CopMove16(cp, bpl2mod, fg_bplmod);
  }

  /* Setup bitplane display delay in BPLCON1 */
  {
    short fg_sh = 15 - (fg_x & 15);
    short bg_sh = 15 - (bg_x & 15);
    CopMove16(cp, bplcon1, (fg_sh << 4) | bg_sh);
  }

  {
    short wrap_bg = -32768;
    short wrap_fg = -32768;
    short y;

    if (bg_y + HEIGHT >= background.height - 1)
      wrap_bg = background.height - bg_y - 1;
    if (fg_y + HEIGHT >= foreground.height - 1)
      wrap_fg = foreground.height - fg_y - 1;

    for (y = 0; y < HEIGHT; y++) {
      if ((wrap_bg - 1 == y) || (wrap_fg - 1 == y)) 
        CopWaitSafe(cp, Y(y), 0);
      if (wrap_bg - 1 == y)
        CopMove16(cp, bpl1mod,
                  -background.bplSize + bg_bplmod + background.bytesPerRow);
      if (wrap_fg - 1 == y)
        CopMove16(cp, bpl2mod,
                  -foreground.bplSize + fg_bplmod + foreground.bytesPerRow);

      if ((wrap_bg == y) || (wrap_fg == y)) 
        CopWaitSafe(cp, Y(y), 0);
      if (wrap_bg == y)
        CopMove16(cp, bpl1mod, bg_bplmod);
      if (wrap_fg == y)
        CopMove16(cp, bpl2mod, fg_bplmod);
    }
  }

  CopEnd(cp);
}

static void Init(void) {
  EnableDMA(DMAF_BLITTER | DMAF_BLITHOG);

  cp0 = NewCopList(500);
  cp1 = NewCopList(500);
  MakeCopperList(cp0);
  CopListActivate(cp0);
  EnableDMA(DMAF_RASTER);
}

static void Kill(void) {
  DisableDMA(DMAF_RASTER | DMAF_BLITTER | DMAF_BLITHOG);
  DeleteCopList(cp0);
  DeleteCopList(cp1);
}

PROFILE(MakeCopperList);

static void Render(void) {
  short bg_h = background.height / 2 - 1;
  short fg_h = foreground.height / 2 - 1;
  short bg_w = background.width / 2;
  short fg_w = foreground.width / 2;

  bg_y = normfx(SIN(frameCount * 12) * bg_h) + bg_h;
  fg_y = normfx(COS(frameCount * 12) * fg_h) + fg_h;
  bg_x = normfx(COS(frameCount * 12) * bg_w) + bg_w;
  fg_x = normfx(SIN(frameCount * 12) * fg_w) + fg_w;

  ProfilerStart(MakeCopperList);
  MakeCopperList(cp1);
  ProfilerStop(MakeCopperList);

  CopListRun(cp1);
  TaskWaitVBlank();
  swapr(cp0, cp1);
}

EFFECT(credits, NULL, NULL, Init, Kill, Render);
