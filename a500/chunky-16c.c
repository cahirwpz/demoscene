#include "startup.h"
#include "blitter.h"
#include "coplist.h"
#include "memory.h"
#include "tga.h"
#include "print.h"

#define WIDTH 160
#define HEIGHT 128

static PixmapT *chunky;
static BitmapT *screen;
static CopListT *cp;
static CopInsT *bpls[6];

static void PixmapScramble(PixmapT *image, PixmapT *chunky) {
  UBYTE *src, *dst;
  ULONG *data;
  LONG n;

  /* [0 0 0 0 a0 a1 a2 a3] [0 0 0 0 b0 b1 b2 b3] => [a2 a3 b2 b3 a0 a1 b0 b1] */
  src = image->pixels;
  dst = chunky->pixels;
  n = chunky->width * chunky->height / 2;

  do {
    BYTE a = *src++;
    BYTE b = *src++;
    *dst++ = (a & 0x0c) | ((b & 0x0c) >> 2) | ((a & 0x03) << 6) | ((b & 0x03) << 4);
  } while (--n);

  /* [ab] [cd] [ef] [gh] => [ab] [ef] [cd] [gh] */
  data = (LONG *)chunky->pixels;
  n = chunky->width * chunky->height / (2 * sizeof(LONG));

  do {
    LONG p = *data;
    *data++ = (p & 0xff0000ff) | ((p & 0x00ff0000) >> 8) | ((p & 0x0000ff00) << 8);
  } while (--n);
}

static void Load() {
  UWORD i;

  cp = NewCopList(4096);
  screen = NewBitmap(WIDTH * 2, HEIGHT * 2, 4);

  {
    PixmapT *pixmap = LoadTGA("data/helmet.tga", PM_CMAP, MEMF_PUBLIC);

    chunky = NewPixmap(pixmap->width, pixmap->height, PM_GRAY4, MEMF_CHIP);
    chunky->palette = pixmap->palette;

    PixmapScramble(pixmap, chunky);
    DeletePixmap(pixmap);
  }

  CopInit(cp);
  CopMakePlayfield(cp, bpls, screen, screen->depth);
  CopMakeDispWin(cp, X(0), Y(0), screen->width, screen->height);
  CopLoadPal(cp, chunky->palette, 0);
  for (i = 16; i < 32; i++)
    CopSetRGB(cp, i, 0x000);
  for (i = 0; i < HEIGHT * 2; i++) {
    CopWaitMask(cp, Y(i), 0, 0xff, 0);
    CopMove16(cp, bpl1mod, (i & 1) ? -40 : 0);
    CopMove16(cp, bpl2mod, (i & 1) ? -40 : 0);
  }
  CopEnd(cp);

  CopInsSet32(bpls[0], screen->planes[0]);
  CopInsSet32(bpls[1], screen->planes[0]);
  CopInsSet32(bpls[2], screen->planes[2]);
  CopInsSet32(bpls[3], screen->planes[2]);
}

static void UnLoad() {
  DeletePalette(chunky->palette);
  DeletePixmap(chunky);
  DeleteBitmap(screen);
  DeleteCopList(cp);
}

static void InitChunkyToPlanar() {
  custom->bltafwm = -1;
  custom->bltalwm = -1;
}

static void ChunkyToPlanar() {
  /* Swap 4x2, pass 1. */
  custom->bltapt = chunky->pixels;
  custom->bltbpt = chunky->pixels + 2;
  custom->bltdpt = screen->planes[0];

  custom->bltamod = 2;
  custom->bltbmod = 2;
  custom->bltdmod = 0;
  custom->bltcdat = 0xf0f0;

  custom->bltcon0 = (SRCA | SRCB | DEST) | (ABC | ABNC | ANBC | NABNC);
  custom->bltcon1 = 4 << BSHIFTSHIFT;

#ifdef OCS
  custom->bltsize = 1; // (1024 << 6)
  WaitBlitter();
  custom->bltsize = 1; // (1024 << 6)
  WaitBlitter();
  custom->bltsize = 1 | (512 << 6);
  WaitBlitter();
#else
  custom->bltsizv = WIDTH * HEIGHT / 8;
  custom->bltsizh = 1;
  WaitBlitter();
#endif

  /* Swap 4x2, pass 2. */
  // custom->bltapt = chunky->pixels + WIDTH * HEIGHT / 2;
  // custom->bltbpt = chunky->pixels + WIDTH * HEIGHT / 2 + 2;
  custom->bltdpt = screen->planes[2] + WIDTH * HEIGHT / 4;

  custom->bltcon0 = (SRCA | SRCB | DEST) | (ABC | ABNC | ANBC | NABNC) | (4 << ASHIFTSHIFT);
  custom->bltcon1 = BLITREVERSE;

#ifdef OCS
  custom->bltsize = 1; // | (1024 << 6)
  WaitBlitter();
  custom->bltsize = 1; // | (1024 << 6)
  WaitBlitter();
  custom->bltsize = (513 << 6) | 1;
  WaitBlitter();
#else
  custom->bltsizv = WIDTH * HEIGHT / 8 + 2;
  custom->bltsizh = 1;
  WaitBlitter();
#endif
}

static void Init() {
  CopListActivate(cp);
  custom->dmacon = DMAF_SETCLR | DMAF_RASTER | DMAF_BLITTER;

  InitChunkyToPlanar();
}

static void Render() {
  LONG lines = ReadLineCounter();
  ChunkyToPlanar();
  Log("c2p: %ld\n", ReadLineCounter() - lines);

  WaitVBlank();
}

EffectT Effect = { Load, UnLoad, Init, NULL, Render };
