#include "effect.h"
#include "copper.h"
#include "color.h"
#include "blitter.h"
#include "fx.h"

#define WIDTH   320
#define HEIGHT  256
#define DEPTH   2

static __code BitmapT *screen;
static __code CopInsPairT *bplptr;
static __code CopInsT *bplshift;
static __code CopInsT *midpoint;
static __code CopListT *cp;

#include "data/ghostown-logo.c"
#include "data/stardust-1.c"
#include "data/stardust-2.c"
#include "data/stardust-3.c"
#include "data/stardust-4.c"
#include "data/stardust-5.c"
#include "data/stardust-6.c"

static const BitmapT *stardust[6] = {
  &stardust_1,
  &stardust_2,
  &stardust_3,
  &stardust_4,
  &stardust_5,
  &stardust_6,
};

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

static void ChangePalette(short s) {
  short b0 = background_colors[0];
  short b1 = background_colors[1];
  short b2 = background_colors[2];
  short b3 = background_colors[3];
  short f1 = logo_colors[1];
  short f2 = logo_colors[2];
  short f3 = logo_colors[3];

  /*    C  F  B */
  /* 0000  0  0 */
  SetColor(0, b0);
  /* 0001  0  1 */
  SetColor(1, b1);
  /* 0010  1  0 */
  SetColor(2, ColorTransition(b0, f1, s));
  /* 0011  1  1 */
  SetColor(3, ColorTransition(b1, f1, s));
  /* 0100  0  2 */
  SetColor(4, b2);
  /* 0101  0  3 */
  SetColor(5, b3);
  /* 0110  1  2 */
  SetColor(6, ColorTransition(b2, f1, s));
  /* 0111  1  3 */
  SetColor(7, ColorTransition(b3, f1, s));
  /* 1000  2  0 */
  SetColor(8, ColorTransition(b0, f2, s));
  /* 1001  2  1 */
  SetColor(9, ColorTransition(b1, f2, s));
  /* 1010  3  0 */
  SetColor(10, ColorTransition(b0, f3, s));
  /* 1011  3  1 */
  SetColor(11, ColorTransition(b1, f3, s));
  /* 1100  2  2 */
  SetColor(12, ColorTransition(b2, f2, s));
  /* 1101  2  3 */
  SetColor(13, ColorTransition(b3, f2, s));
  /* 1110  3  2 */
  SetColor(14, ColorTransition(b2, f3, s));
  /* 1111  3  3 */
  SetColor(15, ColorTransition(b3, f3, s));
}

static CopListT *MakeCopperList(void) {
  CopListT *cp = NewCopList(30);

  bplptr = CopInsPtr(cp);
  CopMove32(cp, bplpt[0], stardust_1.planes[0]);
  CopMove32(cp, bplpt[1], screen->planes[0]);
  CopMove32(cp, bplpt[2], stardust_1.planes[1]);
  CopMove32(cp, bplpt[3], screen->planes[1]);
  CopMove16(cp, bpl1mod, stardust_1_bytesPerRow - 320 / 8);
  CopMove16(cp, bpl2mod, 0);

  bplshift = CopInsPtr(cp);
  CopMove16(cp, bplcon1, 0);

  midpoint = CopInsPtr(cp);
  CopWait(cp, Y(stardust_1_height) - 1, 0);
  CopMove16(cp, bpl1mod, -stardust_1_bytesPerRow - 320 / 8);
  CopListFinish(cp);

  return cp;
}

static void Init(void) {
  screen = NewBitmap(WIDTH, HEIGHT, DEPTH, BM_CLEAR);

  SetupMode(MODE_LORES, stardust_1_depth + DEPTH);
  SetupBitplaneFetch(MODE_LORES, X(0), WIDTH);
  SetupDisplayWindow(MODE_LORES, X(16), Y(0), WIDTH-16, HEIGHT);

  ChangePalette(8);
  cp = MakeCopperList();

  CopListActivate(cp);
  EnableDMA(DMAF_RASTER | DMAF_BLITTER);
}

static void Kill(void) {
  CopperStop();
  DisableDMA(DMAF_RASTER | DMAF_BLITTER);
  DeleteBitmap(screen);
  DeleteCopList(cp);
}

static void Render(void) {
  {
    short xo = normfx(SIN(frameCount * 8) * 32);
    short yo = normfx(SIN(frameCount * 16) * 32);

    BitplaneCopyFast(screen, 0, 80 + xo, 64 + yo, &logo, 0);
    BitplaneCopyFast(screen, 1, 80 + xo, 64 + yo, &logo, 1);
  }
 
  {
    short s = normfx(SIN(frameCount * 64) * 6) + 8;
    ChangePalette(s);
  }

  {
    short f = mod16(frameCount >> 1, 6);
    short xo = normfx(SIN(frameCount * 32) * 64) + 64;
    short yo = normfx(COS(frameCount * 32) * 64) + 64;
    int offset = ((xo >> 3) & -2) + yo * stardust_1_bytesPerRow;

    CopInsSet16(bplshift, ~xo & 15);
    CopInsSet32(&bplptr[0], stardust[f]->planes[0] + offset);
    CopInsSet32(&bplptr[2], stardust[f]->planes[1] + offset);
    midpoint->wait.vp = Y(stardust_1_height) - 1 - yo;
  }

  TaskWaitVBlank();
}

EFFECT(Transparency, NULL, NULL, Init, Kill, Render, NULL);
