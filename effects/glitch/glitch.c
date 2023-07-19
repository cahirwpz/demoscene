#include "effect.h"
#include "copper.h"
#include "gfx.h"
#include "blitter.h"

#define WIDTH (160 + 32)
#define HEIGHT (128 + 32)
#define XSTART ((320 - WIDTH) / 2)
#define YSTART ((256 - HEIGHT) / 2)
#define DEPTH 3

static BitmapT *screen[2];
static short active = 0;
static CopInsPairT *bplptr;
static CopListT *cp;
static CopInsT *line[HEIGHT];

#include "data/ghostown-logo.c"

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
  short i;

  screen[0] = NewBitmap(WIDTH, HEIGHT, DEPTH);
  screen[1] = NewBitmap(WIDTH, HEIGHT, DEPTH);

  SetupPlayfield(MODE_LORES, DEPTH, X(XSTART), Y(YSTART), WIDTH, HEIGHT);
  SetColor(0, 0x000);
  SetColor(1, 0x00F);
  SetColor(2, 0x0F0);
  SetColor(3, 0xF00);
  SetColor(4, 0x0FF);
  SetColor(5, 0xF0F);
  SetColor(6, 0xFF0);
  SetColor(7, 0xFFF);

  cp = NewCopList(100 + HEIGHT * 2);
  CopInit(cp);
  bplptr = CopSetupBitplanes(cp, screen[active], DEPTH);
  for (i = 0; i < HEIGHT; i++) {
    CopWait(cp, Y(YSTART + i), 0);
    /* Alternating shift by one for bitplane data. */
    line[i] = CopMove16(cp, bplcon1, 0);
  }
  CopEnd(cp);

  CopListActivate(cp);
  EnableDMA(DMAF_RASTER | DMAF_BLITTER);
}

static void Kill(void) {
  DisableDMA(DMAF_COPPER | DMAF_RASTER | DMAF_BLITTER);

  DeleteCopList(cp);
  DeleteBitmap(screen[0]);
  DeleteBitmap(screen[1]);
}

static inline u_short random(void) {
  static u_int seed = 0xDEADC0DE;

  asm("rol.l  %0,%0\n"
      "addq.l #5,%0\n"
      : "+d" (seed));

  return seed;
}

PROFILE(RenderGlitch);

static void Render(void) {
  short i;

  ProfilerStart(RenderGlitch);

  if (RightMouseButton()) {
    int x1 = (random() % 5) - 2;
    int x2 = (random() % 5) - 2;
    int y1 = (random() % 5) - 2;
    int y2 = (random() % 5) - 2;

    BitplaneCopyFast(screen[active], 0, 16 + x1, 16 + y1, &logo, 0);
    BitplaneCopyFast(screen[active], 1, 16 + x2, 16 + y2, &logo, 0);
    BitplaneCopyFast(screen[active], 2, 16, 16, &logo, 0);

    for (i = 0; i < HEIGHT; i++)
      CopInsSet16(line[i], 0);
  } else {

    BitplaneCopyFast(screen[active], 0, 16, 16, &logo, 0);
    BitplaneCopyFast(screen[active], 1, 16, 16, &logo, 0);
    BitplaneCopyFast(screen[active], 2, 16, 16, &logo, 0);

    for (i = 0; i < HEIGHT; i++) {
      short shift = random() % 3;
      CopInsSet16(line[i], (shift << 4) | shift);
    }
  }

  ProfilerStop(RenderGlitch);

  ITER(i, 0, DEPTH - 1, CopInsSet32(&bplptr[i], screen[active]->planes[i]));
  TaskWaitVBlank();
  active ^= 1;
}

EFFECT(Glitch, NULL, NULL, Init, Kill, Render, NULL);
