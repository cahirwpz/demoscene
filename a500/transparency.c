#include "startup.h"
#include "hardware.h"
#include "coplist.h"
#include "gfx.h"
#include "color.h"
#include "bltop.h"
#include "ilbm.h"
#include "fx.h"

STRPTR __cwdpath = "data";

#define WIDTH   320
#define HEIGHT  256
#define DEPTH   5

static BitmapT *screen;
static BitmapT *background;
static BitmapT *logo;
static CopListT *cp;
static CopInsT *pal;

static UWORD pal1[8];
static UWORD pal2[4];

static void Load() {
  background = LoadILBM("transparency-bg.ilbm");
  logo = LoadILBM("ghostown_160x128_4col.iff");

  {
    WORD i;

    for (i = 0; i < 8; i++) {
      ColorT *c = &background->palette->colors[i];
      pal1[i] = ((c->r & 0xf0) << 4) | (c->g & 0xf0) | ((c->b & 0xf0) >> 4);
    }

    for (i = 0; i < 4; i++) {
      ColorT *c = &logo->palette->colors[i];
      pal2[i] = ((c->r & 0xf0) << 4) | (c->g & 0xf0) | ((c->b & 0xf0) >> 4);
    }
  }

}

static void UnLoad() {
  DeletePalette(logo->palette);
  DeleteBitmap(logo);
  DeletePalette(background->palette);
  DeleteBitmap(background);
}

static void BitplaneCopyFast(BitmapT *dst, WORD d, UWORD x, UWORD y,
                             BitmapT *src, WORD s)
{
  APTR srcbpt = src->planes[s];
  APTR dstbpt = dst->planes[d] + ((x & ~15) >> 3) + y * dst->bytesPerRow;
  UWORD dstmod = dst->bytesPerRow - src->bytesPerRow;
  UWORD bltsize = (src->height << 6) | (src->bytesPerRow >> 1);
  UWORD bltshift = rorw(x & 15, 4);

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

static void Init() {
  screen = NewBitmap(WIDTH, HEIGHT, DEPTH);

  custom->dmacon = DMAF_SETCLR | DMAF_BLITTER;
  BitmapCopy(screen, 0, 0, background);

  cp = NewCopList(100);
  CopInit(cp);
  CopSetupGfxSimple(cp, MODE_LORES, DEPTH, X(0), Y(0), WIDTH, HEIGHT);
  CopSetupBitplanes(cp, NULL, screen, DEPTH);
  CopLoadPal(cp, background->palette, 0);
  pal = CopLoadColor(cp, 8, 31, 0);
  CopEnd(cp);

  CopListActivate(cp);
  custom->dmacon = DMAF_SETCLR | DMAF_RASTER;
}

static void Kill() {
  DeleteBitmap(screen);
  DeleteCopList(cp);
}

static void Render() {
  WORD xo = normfx(SIN(frameCount * 8) * 32);
  WORD yo = normfx(SIN(frameCount * 16) * 32);
  WORD s = normfx(SIN(frameCount * 64) * 6) + 8;
  WORD i;

  BitplaneCopyFast(screen, 3, 80 + xo, 64 + yo, logo, 0);
  BitplaneCopyFast(screen, 4, 80 + xo, 64 + yo, logo, 1);
  
  for (i = 0; i < 24; i++)
    CopInsSet16(pal + i, ColorTransition(pal1[i & 7], pal2[i / 8 + 1], s));
}

EffectT Effect = { Load, UnLoad, Init, Kill, Render };
