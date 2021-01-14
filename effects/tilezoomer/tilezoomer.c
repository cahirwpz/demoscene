#include "effect.h"
#include "blitter.h"
#include "copper.h"
#include "memory.h"
#include <stdlib.h>

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
static short tiles[(TILES - 1) * (TILES -1) * 4];

static void CalculateTiles(void) { 
  short *tile = tiles;
  short x, y;

  for (y = 0; y < TILES - 1; y++) {
    for (x = 0; x < TILES - 1; x++) {
      short xo = x - TILESIZE / 2;
      short yo = (TILES - y) - TILESIZE / 2;
      short dy = y * TILESIZE;
      short dx = x * TILESIZE;
      short sx = dx + yo * ROTATION - xo * ZOOM;
      short sy = dy + xo * ROTATION + yo * ZOOM;

      *tile++ = sx;
      *tile++ = dx;
      *tile++ = sy * WIDTH / 8;
      *tile++ = dy * WIDTH / 8;
    }
  }
}

static void Init(void) {
  CalculateTiles();

  screen0 = NewBitmap(WIDTH, HEIGHT, DEPTH);
  screen1 = NewBitmap(WIDTH, HEIGHT, DEPTH);

  SetupPlayfield(MODE_LORES, DEPTH, X(32), Y(0), 256, 256);
  SetColor(0, 0x000);
  SetColor(1, 0x44f);
  SetColor(2, 0x88f);
  SetColor(3, 0xccf);

  cp = NewCopList(100);
  CopInit(cp);
  CopSetupBitplanes(cp, bplptr, screen0, DEPTH);
  CopMove16(cp, bpl1mod, 4);
  CopMove16(cp, bpl2mod, 4);
  CopEnd(cp);

  CopListActivate(cp);
  EnableDMA(DMAF_RASTER | DMAF_BLITTER | DMAF_BLITHOG);
}

static void Kill(void) {
  DeleteBitmap(screen0);
  DeleteBitmap(screen1);
  DeleteCopList(cp);
}

static void DrawSeed(void) {
  u_char *bpl0 = screen0->planes[0];
  u_char *bpl1 = screen0->planes[1];
  int offset = ((HEIGHT / 2 + 2 * TILESIZE - TILESIZE / 4) * WIDTH + (WIDTH / 2 - TILESIZE / 2)) / 8;
  short n = 8;

  while (--n >= 0) {
    bpl0[offset] = random();
    bpl1[offset] = random();
    offset += WIDTH / 8;
  }
}

#define BLTMOD (WIDTH / 8 - TILESIZE / 8 - 2)
#define BLTSIZE ((TILESIZE << 6) | ((TILESIZE + 16) >> 4))

static void MoveTiles(void) {
  void *src = screen0->planes[0];
  void *dst = screen1->planes[0];
  short xshift = random() & (TILESIZE - 1);
  short yshift = random() & (TILESIZE - 1);
  short n = (TILES - 1) * (TILES - 1) - 1;
  short *tile = tiles;

  custom->bltadat = 0xffff;
  custom->bltbmod = BLTMOD;
  custom->bltcmod = BLTMOD;
  custom->bltdmod = BLTMOD;
  custom->bltcon0 = (SRCB | SRCC | DEST) | (ABC | ABNC | NABC | NANBC);

  yshift *= WIDTH / 8;

  do {
    short sx = *tile++ + xshift;
    short dx = *tile++ + xshift;
    short sy = *tile++ + yshift + ((sx >> 3) & ~1);
    short dy = *tile++ + yshift + ((dx >> 3) & ~1);
    void *srcpt = src + sy;
    void *dstpt = dst + dy;
    u_short bltcon1;
    u_int mask;
    short shift;

    sx &= 15; dx &= 15; shift = dx - sx;

    if (shift >= 0) {
      bltcon1 = BSHIFT(shift);
      mask = 0xffff0000;
    } else {
      bltcon1 = BSHIFT(-shift) | BLITREVERSE;
      mask = 0x0000ffff;

      srcpt += WIDTH * (TILESIZE - 1) / 8 + 2;
      dstpt += WIDTH * (TILESIZE - 1) / 8 + 2;
    }

    asm("ror.l %1,%0" : "+d" (mask) : "d" (dx));

    WaitBlitter();

    custom->bltcon1 = bltcon1;
    custom->bltafwm = mask >> 16;
    custom->bltalwm = mask;
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

PROFILE(TileZoomer);

static void Render(void) {
  ProfilerStart(TileZoomer);
  {
    DrawSeed();
    MoveTiles();
  }
  ProfilerStop(TileZoomer);

  CopInsSet32(bplptr[0], screen1->planes[0] + 2 + WIDTH * TILESIZE / 8);
  CopInsSet32(bplptr[1], screen1->planes[1] + 2 + WIDTH * TILESIZE / 8);
  TaskWaitVBlank();
  swapr(screen0, screen1);
}

EFFECT(tilezoomer, NULL, NULL, Init, Kill, Render);
