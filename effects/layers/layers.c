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
#include "data/bg-gradient.c"
#include "data/fg-gradient.c"

static CopListT *cp0, *cp1;
static short fg_y, bg_y, fg_x, bg_x;

#define bg_bplmod ((background_width - (WIDTH + 16)) / 8)
#define fg_bplmod ((foreground_width - (WIDTH + 16)) / 8)

static void SetupLayers(CopListT *cp) {
  short fg_sh = 15 - (fg_x & 15);
  short bg_sh = 15 - (bg_x & 15);
  short i;

  /* Setup BPLPT for even bitplanes and BPL1MOD (background) */
  for (i = 0; i < background.depth; i++) {
    int offset = mul16(bg_y, background_bytesPerRow) + (bg_x & -16) / 8;
    CopMove32(cp, bplpt[i * 2], background.planes[i] + offset);
  }
  CopMove16(cp, bpl1mod, bg_bplmod);

  /* Setup BPLPT for odd bitplanes and BPL2MOD (background) */
  for (i = 0; i < foreground.depth; i++) {
    int offset = mul16(fg_y, foreground_bytesPerRow) + (fg_x & -16) / 8;
    CopMove32(cp, bplpt[i * 2 + 1], foreground.planes[i] + offset);
  }
  CopMove16(cp, bpl2mod, fg_bplmod);

  /* Setup bitplane display delay in BPLCON1 */
  CopMove16(cp, bplcon1, (fg_sh << 4) | bg_sh);
}

#define STEP 8
#define COLORS 8

static void SetupRaster(CopListT *cp) {
  short *bg_pal = bg_gradient_pixels;
  short *fg_pal = fg_gradient_pixels;
  short bg_pal_y = mod16(bg_y / STEP, bg_gradient_height);
  short fg_pal_y = mod16(fg_y / STEP, fg_gradient_height);
  short wrap_bg = -1;
  short wrap_fg = -1;
  short y, _bg_y, _fg_y;

  bg_pal += bg_pal_y * bg_gradient_width + 1;
  fg_pal += fg_pal_y * fg_gradient_width + 1;

  CopSetColor(cp, 0, 0);
  CopSetColor(cp, 1, *bg_pal++);
  CopSetColor(cp, 2, *bg_pal++);
  CopSetColor(cp, 3, *bg_pal++);
  CopSetColor(cp, 4, *bg_pal++);
  CopSetColor(cp, 5, *bg_pal++);
  CopSetColor(cp, 6, *bg_pal++);
  CopSetColor(cp,  9, *fg_pal++);
  CopSetColor(cp, 10, *fg_pal++);
  CopSetColor(cp, 11, *fg_pal++);
  CopSetColor(cp, 12, *fg_pal++);
  CopSetColor(cp, 13, *fg_pal++);
  bg_pal_y++, fg_pal_y++;

  if (bg_y + HEIGHT >= background_height - 1)
    wrap_bg = background_height - bg_y - 1;
  if (fg_y + HEIGHT >= foreground_height - 1)
    wrap_fg = foreground_height - fg_y - 1;

  for (y = 0, _bg_y = bg_y, _fg_y = fg_y; y < HEIGHT; y++, _bg_y++, _fg_y++) {
    u_char f = 0;

    if (y == (short)(wrap_bg - 1))
      f |= 1;
    if (y == (short)(wrap_fg - 1))
      f |= 2;
    if (y == wrap_bg)
      f |= 4;
    if (y == wrap_fg)
      f |= 8;
    if (!(_bg_y & 7))
      f |= 16;
    if (!(_fg_y & 7))
      f |= 32;

    if (!f)
      continue;

    CopWaitSafe(cp, Y(y), 0);

    if (f & 1)
      CopMove16(cp, bpl1mod,
                -background_bplSize + bg_bplmod + background_bytesPerRow);
    if (f & 2)
      CopMove16(cp, bpl2mod,
                -foreground_bplSize + fg_bplmod + foreground_bytesPerRow);

    if (f & 4)
      CopMove16(cp, bpl1mod, bg_bplmod);
    if (f & 8)
      CopMove16(cp, bpl2mod, fg_bplmod);

    if (y == 0)
      continue;

    /* XXX: swapping too many colors in a single line
     * results in glitches near to the bottom of raster */
    if (f & 16) {
      if (bg_pal_y >= bg_gradient_height) {
        bg_pal = bg_gradient_pixels;
        bg_pal_y = 0;
      }
      bg_pal++, bg_pal_y++;
      CopSetColor(cp, 1, *bg_pal++);
      CopSetColor(cp, 2, *bg_pal++);
      CopSetColor(cp, 3, *bg_pal++);
      CopSetColor(cp, 4, *bg_pal++);
      CopSetColor(cp, 5, *bg_pal++);
      CopSetColor(cp, 6, *bg_pal++);
    }

    if (f & 32) {
      if (fg_pal_y >= fg_gradient_height) {
        fg_pal = fg_gradient_pixels;
        fg_pal_y = 0;
      }
      fg_pal++, fg_pal_y++;
      CopSetColor(cp,  9, *fg_pal++);
      CopSetColor(cp, 10, *fg_pal++);
      CopSetColor(cp, 11, *fg_pal++);
      CopSetColor(cp, 12, *fg_pal++);
      CopSetColor(cp, 13, *fg_pal++);
    }
  }
}

static void MakeCopperList(CopListT *cp) {

  CopInit(cp);
  CopSetupDisplayWindow(cp, MODE_LORES, X(0), Y(0), WIDTH, HEIGHT);
  CopSetupBitplaneFetch(cp, MODE_LORES, X(-16), WIDTH + 16);
  CopSetupMode(cp, MODE_DUALPF, DEPTH);
  /* Reverse playfields priorities (for testing) */
#if 0
  CopMove16(cp, bplcon2, 0);
#endif

  SetupLayers(cp);
  SetupRaster(cp);
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