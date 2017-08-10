#include "startup.h"
#include "blitter.h"
#include "coplist.h"
#include "memory.h"
#include "random.h"

/* Add tile sized margins on every side to hide visual artifacts. */
#define WIDTH   (256 + 32)
#define HEIGHT  (256 + 32)
#define DEPTH   2

#define TILESIZE  16
#define TILES     (WIDTH / TILESIZE)
#define ROTATION  1
#define ZOOM      1

static BitmapT *screen0, *screen1;
static CopListT *cp;
static CopInsT *bplptr[DEPTH];
static WORD tiles[(TILES - 1) * (TILES -1) * 4];

static void CalculateTiles() { 
  WORD *tile = tiles;
  WORD x, y;

  for (y = 0; y < TILES - 1; y++) {
    for (x = 0; x < TILES - 1; x++) {
      WORD xo = x - TILESIZE / 2;
      WORD yo = (TILES - y) - TILESIZE / 2;
      WORD dy = y * TILESIZE;
      WORD dx = x * TILESIZE;
      WORD sx = dx + yo * ROTATION - xo * ZOOM;
      WORD sy = dy + xo * ROTATION + yo * ZOOM;

      *tile++ = sx;
      *tile++ = dx;
      *tile++ = sy * WIDTH / 8;
      *tile++ = dy * WIDTH / 8;
    }
  }
}

static void Init() {
  CalculateTiles();

  screen0 = NewBitmap(WIDTH, HEIGHT, DEPTH);
  screen1 = NewBitmap(WIDTH, HEIGHT, DEPTH);

  EnableDMA(DMAF_BLITTER | DMAF_BLITHOG);

  cp = NewCopList(100);
  CopInit(cp);
  CopSetupGfxSimple(cp, MODE_LORES, DEPTH, X(32), Y(0), 256, 256);
  CopSetupBitplanes(cp, bplptr, screen0, DEPTH);
  CopMove16(cp, bpl1mod, 4);
  CopMove16(cp, bpl2mod, 4);
  CopSetRGB(cp, 0, 0x000);
  CopSetRGB(cp, 1, 0x44f);
  CopSetRGB(cp, 2, 0x88f);
  CopSetRGB(cp, 3, 0xccf);
  CopEnd(cp);

  CopListActivate(cp);
  EnableDMA(DMAF_RASTER);
}

static void Kill() {
  DeleteBitmap(screen0);
  DeleteBitmap(screen1);
  DeleteCopList(cp);
}

static void DrawSeed() {
  UBYTE *bpl0 = screen0->planes[0];
  UBYTE *bpl1 = screen0->planes[1];
  LONG offset = ((HEIGHT / 2 + 2 * TILESIZE - TILESIZE / 4) * WIDTH + (WIDTH / 2 - TILESIZE / 2)) / 8;
  WORD n = 8;

  while (--n >= 0) {
    bpl0[offset] = random();
    bpl1[offset] = random();
    offset += WIDTH / 8;
  }
}

#define BLTMOD (WIDTH / 8 - TILESIZE / 8 - 2)
#define BLTSIZE ((TILESIZE << 6) | ((TILESIZE + 16) >> 4))

static void MoveTiles() {
  APTR src = screen0->planes[0];
  APTR dst = screen1->planes[0];
  WORD xshift = random() & (TILESIZE - 1);
  WORD yshift = random() & (TILESIZE - 1);
  WORD n = (TILES - 1) * (TILES - 1) - 1;
  WORD *tile = tiles;

  custom->bltadat = 0xffff;
  custom->bltbmod = BLTMOD;
  custom->bltcmod = BLTMOD;
  custom->bltdmod = BLTMOD;
  custom->bltcon0 = (SRCB | SRCC | DEST) | (ABC | ABNC | NABC | NANBC);

  yshift *= WIDTH / 8;

  do {
    WORD sx = *tile++ + xshift;
    WORD dx = *tile++ + xshift;
    WORD sy = *tile++ + yshift + ((sx >> 3) & ~1);
    WORD dy = *tile++ + yshift + ((dx >> 3) & ~1);
    APTR srcpt = src + sy;
    APTR dstpt = dst + dy;
    UWORD bltcon1;
    ULONG mask;
    WORD shift;

    sx &= 15; dx &= 15; shift = dx - sx;

    if (shift >= 0) {
      bltcon1 = shift << BSHIFTSHIFT;
      mask = 0xffff0000;
    } else {
      bltcon1 = (-shift << BSHIFTSHIFT) | BLITREVERSE;
      mask = 0x0000ffff;

      srcpt += WIDTH * (TILESIZE - 1) / 8 + 2;
      dstpt += WIDTH * (TILESIZE - 1) / 8 + 2;
    }

    asm("ror.l %1,%0" : "+d" (mask) : "d" (dx));

    WaitBlitter();

    custom->bltcon1 = bltcon1;
    *(ULONG *)&custom->bltafwm = mask;
    custom->bltcpt = dstpt;
    custom->bltbpt = srcpt;
    custom->bltdpt = dstpt;
    custom->bltsize = BLTSIZE;

    srcpt += WIDTH * HEIGHT / 8;
    dstpt += WIDTH * HEIGHT / 8;

    WaitBlitter();
    custom->bltcpt = dstpt;
    custom->bltbpt = srcpt;
    custom->bltdpt = dstpt;
    custom->bltsize = BLTSIZE;
  } while (--n >= 0);
}

static void Render() {
  // LONG lines = ReadLineCounter();
  DrawSeed();
  MoveTiles();
  // Log("tilezoomer: %ld\n", ReadLineCounter() - lines);

  WaitVBlank();
  CopInsSet32(bplptr[0], screen1->planes[0] + 2 + WIDTH * TILESIZE / 8);
  CopInsSet32(bplptr[1], screen1->planes[1] + 2 + WIDTH * TILESIZE / 8);
  swapr(screen0, screen1);
}

EffectT Effect = { NULL, NULL, Init, Kill, Render };
