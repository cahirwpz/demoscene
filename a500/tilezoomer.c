#include "startup.h"
#include "blitter.h"
#include "coplist.h"
#include "memory.h"
#include "random.h"

#define WIDTH   256
#define HEIGHT  256
#define DEPTH   2

#define TILESIZE  16
#define TILES     (256 / TILESIZE)
#define ROTATION  1
#define ZOOM      1

static BitmapT *screen[2];
static CopListT *cp;
static CopInsT *bplptr[DEPTH];
static UWORD active = 0;

static void Init() {
  screen[0] = NewBitmap(WIDTH, HEIGHT, DEPTH);
  screen[1] = NewBitmap(WIDTH, HEIGHT, DEPTH);

  custom->dmacon = DMAF_SETCLR | DMAF_BLITTER | DMAF_BLITHOG;

  cp = NewCopList(100);
  CopInit(cp);
  CopMakePlayfield(cp, bplptr, screen[active], DEPTH);
  CopMakeDispWin(cp, X(32), Y(0), WIDTH, HEIGHT);
  CopSetRGB(cp, 0, 0x000);
  CopSetRGB(cp, 1, 0x44f);
  CopSetRGB(cp, 2, 0x88f);
  CopSetRGB(cp, 3, 0xccf);
  CopEnd(cp);

  CopListActivate(cp);
  custom->dmacon = DMAF_SETCLR | DMAF_RASTER;
}

static void Kill() {
  DeleteBitmap(screen[0]);
  DeleteBitmap(screen[1]);
  DeleteCopList(cp);
}

static void DrawSeed() {
  UBYTE *bpl0 = screen[active]->planes[0];
  UBYTE *bpl1 = screen[active]->planes[1];
  LONG offset = ((HEIGHT / 2 + TILESIZE / 2) * WIDTH + WIDTH / 2 + TILESIZE / 2) / 8;
  WORD n = 8;

  while (--n >= 0) {
    bpl0[offset] = random();
    bpl1[offset] = random();
    offset += WIDTH / 8;
  }
}

#define BLTMOD (WIDTH / 8 - TILESIZE / 8 - 2)
#define BLTSIZE ((TILESIZE << 6) | ((TILESIZE + 16) >> 4))

static void CalculateTiles() {
  APTR src = screen[active]->planes[0];
  APTR dst = screen[active ^ 1]->planes[0];
  WORD shift = random() & (TILESIZE - 1);
  WORD x, y;

  custom->bltadat = 0xffff;
  custom->bltbmod = BLTMOD;
  custom->bltcmod = BLTMOD;
  custom->bltdmod = BLTMOD;
  custom->bltcon0 = (SRCB | SRCC | DEST) | (ABC | ABNC | NABC | NANBC);

  for (y = 1; y < TILES - 2; y++) {
    WORD dy = y * TILESIZE + shift;

    for (x = 1; x < TILES - 2; x++) {
      WORD dx = x * TILESIZE + shift;
      WORD sx = dx + (TILES - y - TILESIZE / 2) * ROTATION - (x - TILESIZE / 2) * ZOOM;
      WORD sy = dy + (x - TILESIZE / 2) * ROTATION + (TILES - y - TILESIZE / 2) * ZOOM;

      APTR srcpt = src + (sx & ~15) / 8 + sy * WIDTH / 8;
      APTR dstpt = dst + (dx & ~15) / 8 + dy * WIDTH / 8;
      UWORD srcshift = sx & 15;
      UWORD dstshift = dx & 15;

      if (srcshift <= dstshift) {
        UWORD shift = dstshift - srcshift;
        UWORD afwm = 0xffff >> dstshift;
        UWORD alwm = 0xffff << (15 - dstshift);
        UWORD bltcon1 = shift << BSHIFTSHIFT;

        WaitBlitter();

        custom->bltcon1 = bltcon1;
        custom->bltalwm = alwm;
        custom->bltafwm = afwm;
        custom->bltbpt = srcpt;
        custom->bltcpt = dstpt;
        custom->bltdpt = dstpt;
        custom->bltsize = BLTSIZE;

        srcpt += WIDTH * HEIGHT / 8;
        dstpt += WIDTH * HEIGHT / 8;

        WaitBlitter();
        custom->bltbpt = srcpt;
        custom->bltcpt = dstpt;
        custom->bltdpt = dstpt;
        custom->bltsize = BLTSIZE;
      } else {
        UWORD shift = srcshift - dstshift;
        UWORD afwm = 0xffff << (15 - dstshift);
        UWORD alwm = 0xffff >> dstshift;
        UWORD bltcon1 = (shift << BSHIFTSHIFT) | BLITREVERSE;

        srcpt += WIDTH * (TILESIZE - 1) / 8 + 2;
        dstpt += WIDTH * (TILESIZE - 1) / 8 + 2;

        WaitBlitter();

        custom->bltcon1 = bltcon1;
        custom->bltalwm = alwm;
        custom->bltafwm = afwm;
        custom->bltbpt = srcpt;
        custom->bltcpt = dstpt;
        custom->bltdpt = dstpt;
        custom->bltsize = BLTSIZE;

        srcpt += WIDTH * HEIGHT / 8;
        dstpt += WIDTH * HEIGHT / 8;

        WaitBlitter();
        custom->bltbpt = srcpt;
        custom->bltcpt = dstpt;
        custom->bltdpt = dstpt;
        custom->bltsize = BLTSIZE;
      }
    }
  }
}

static void Render() {
  LONG lines = ReadLineCounter();
  DrawSeed();
  CalculateTiles();
  Log("tilezoomer: %ld\n", ReadLineCounter() - lines);

  WaitVBlank();
  active ^= 1;
  CopInsSet32(bplptr[0], screen[active]->planes[0]);
  CopInsSet32(bplptr[1], screen[active]->planes[1]);
}

EffectT Effect = { NULL, NULL, Init, Kill, Render };
