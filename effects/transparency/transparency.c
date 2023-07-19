#include "effect.h"
#include "copper.h"
#include "gfx.h"
#include "color.h"
#include "blitter.h"
#include "fx.h"

#define WIDTH   320
#define HEIGHT  256
#define DEPTH   5

static BitmapT *screen;
static CopListT *cp;
static CopInsT *pal;

#include "data/ghostown-logo.c"
#include "data/transparency-bg.c"

#define pal1 background_pal.colors
#define pal2 logo_pal.colors

static void BitplaneCopyFast(BitmapT *dst, short d, u_short x, u_short y,
                             const BitmapT *src, short s)
{
  void *srcbpt = src->planes[s];
  void *dstbpt = dst->planes[d] + ((x & ~15) >> 3) + y * dst->bytesPerRow;
  u_short dstmod = dst->bytesPerRow - src->bytesPerRow;
  u_short bltsize = (src->height << 6) | (src->bytesPerRow >> 1);
  u_short bltshift = rorw(x & 15, 4);

  WaitBlitter();

  if (bltshift) {
    bltsize += 1; dstmod -= 2;

    custom->bltalwm = 0;
    custom->bltamod = -2;
  } else {
    custom->bltalwm = -1;
    custom->bltamod = 0;
  }

  custom->bltdmod = dstmod;
  custom->bltcon0 = (SRCA | DEST | A_TO_D) | bltshift;
  custom->bltcon1 = 0;
  custom->bltafwm = -1;
  custom->bltapt = srcbpt;
  custom->bltdpt = dstbpt;
  custom->bltsize = bltsize;
}

static void Init(void) {
  screen = NewBitmap(WIDTH, HEIGHT, DEPTH);

  EnableDMA(DMAF_BLITTER);
  BitmapCopy(screen, 0, 0, &background);

  SetupPlayfield(MODE_LORES, DEPTH, X(0), Y(0), WIDTH, HEIGHT);
  LoadPalette(&background_pal, 0);

  cp = NewCopList(100);
  CopInit(cp);
  CopSetupBitplanes(cp, screen, DEPTH);
  pal = CopLoadColor(cp, 8, 31, 0);
  CopEnd(cp);

  CopListActivate(cp);
  EnableDMA(DMAF_RASTER);
}

static void Kill(void) {
  DeleteBitmap(screen);
  DeleteCopList(cp);
}

static void Render(void) {
  short xo = normfx(SIN(frameCount * 8) * 32);
  short yo = normfx(SIN(frameCount * 16) * 32);
  short s = normfx(SIN(frameCount * 64) * 6) + 8;
  short i;

  BitplaneCopyFast(screen, 3, 80 + xo, 64 + yo, &logo, 0);
  BitplaneCopyFast(screen, 4, 80 + xo, 64 + yo, &logo, 1);
  
  for (i = 0; i < 24; i++)
    CopInsSet16(pal + i, ColorTransition(pal1[i & 7], pal2[i / 8 + 1], s));

  TaskWaitVBlank();
}

EFFECT(Transparency, NULL, NULL, Init, Kill, Render, NULL);
