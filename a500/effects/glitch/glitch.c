#include "startup.h"
#include "hardware.h"
#include "coplist.h"
#include "gfx.h"
#include "ilbm.h"
#include "blitter.h"
#include "tasks.h"

STRPTR __cwdpath = "data";

#define WIDTH (160 + 32)
#define HEIGHT (128 + 32)
#define XSTART ((320 - WIDTH) / 2)
#define YSTART ((256 - HEIGHT) / 2)
#define DEPTH 3

static BitmapT *screen[2];
static WORD active = 0;
static CopInsT *bplptr[DEPTH];
static BitmapT *logo;
static CopListT *cp;
static CopInsT *line[HEIGHT];

static void Load() {
  logo = LoadILBMCustom("ghostown-logo.ilbm", BM_DISPLAYABLE);
}

static void UnLoad() {
  DeleteBitmap(logo);
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
  WORD i;

  screen[0] = NewBitmap(WIDTH, HEIGHT, DEPTH);
  screen[1] = NewBitmap(WIDTH, HEIGHT, DEPTH);

  cp = NewCopList(100 + HEIGHT * 2);
  CopInit(cp);
  CopSetupGfxSimple(cp, MODE_LORES, DEPTH, X(XSTART), Y(YSTART), WIDTH, HEIGHT);
  CopSetupBitplanes(cp, bplptr, screen[active], DEPTH);
  CopSetRGB(cp, 0, 0x000);
  CopSetRGB(cp, 1, 0x00F);
  CopSetRGB(cp, 2, 0x0F0);
  CopSetRGB(cp, 3, 0xF00);
  CopSetRGB(cp, 4, 0x0FF);
  CopSetRGB(cp, 5, 0xF0F);
  CopSetRGB(cp, 6, 0xFF0);
  CopSetRGB(cp, 7, 0xFFF);
  for (i = 0; i < HEIGHT; i++) {
    CopWait(cp, Y(YSTART + i), 0);
    /* Alternating shift by one for bitplane data. */
    line[i] = CopMove16(cp, bplcon1, 0);
  }
  CopEnd(cp);

  CopListActivate(cp);
  EnableDMA(DMAF_RASTER | DMAF_BLITTER);
}

static void Kill() {
  DisableDMA(DMAF_COPPER | DMAF_RASTER | DMAF_BLITTER);

  DeleteCopList(cp);
  DeleteBitmap(screen[0]);
  DeleteBitmap(screen[1]);
}

static inline UWORD random() {
  static ULONG seed = 0xDEADC0DE;

  asm("rol.l  %0,%0\n"
      "addq.l #5,%0\n"
      : "+d" (seed));

  return seed;
}

static void Render() {
  // LONG lines = ReadLineCounter();
  WORD i;

  if (RightMouseButton()) {
    LONG x1 = (random() % 5) - 2;
    LONG x2 = (random() % 5) - 2;
    LONG y1 = (random() % 5) - 2;
    LONG y2 = (random() % 5) - 2;

    BitplaneCopyFast(screen[active], 0, 16 + x1, 16 + y1, logo, 0);
    BitplaneCopyFast(screen[active], 1, 16 + x2, 16 + y2, logo, 0);
    BitplaneCopyFast(screen[active], 2, 16, 16, logo, 0);

    for (i = 0; i < HEIGHT; i++)
      CopInsSet16(line[i], 0);
  } else {

    BitplaneCopyFast(screen[active], 0, 16, 16, logo, 0);
    BitplaneCopyFast(screen[active], 1, 16, 16, logo, 0);
    BitplaneCopyFast(screen[active], 2, 16, 16, logo, 0);

    for (i = 0; i < HEIGHT; i++) {
      WORD shift = random() % 3;
      CopInsSet16(line[i], (shift << 4) | shift);
    }
  }

  // Log("glitch: %ld\n", ReadLineCounter() - lines);

  ITER(i, 0, DEPTH - 1, CopInsSet32(bplptr[i], screen[active]->planes[i]));
  TaskWait(VBlankEvent);
  active ^= 1;
}

EffectT Effect = { Load, UnLoad, Init, Kill, Render };
