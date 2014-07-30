#include "blitter.h"
#include "coplist.h"
#include "memory.h"
#include "tga.h"
#include "print.h"

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

void Load() {
  UWORD i;

  cp = NewCopList(4096);
  screen = NewBitmap(320, 256, 5, FALSE);
  memset(screen->planes[4], 0xaa, 320 * 256 / 8);

  {
    PixmapT *pixmap = LoadTGA("data/helmet.tga", PM_CMAP);

    chunky = NewPixmap(pixmap->width, pixmap->height, PM_GRAY4, MEMF_CHIP);
    chunky->palette = pixmap->palette;

    PixmapScramble(pixmap, chunky);
    DeletePixmap(pixmap);
  }

  CopInit(cp);
  CopMakePlayfield(cp, bpls, screen);
  CopMakeDispWin(cp, 0x81, 0x2c, screen->width, screen->height);
  CopLoadPal(cp, chunky->palette, 0);
  for (i = 16; i < 32; i++)
    CopSetRGB(cp, i, 0x000);
  for (i = 0; i < 256; i++) {
    CopWait(cp, 0x2c + i, 0);
    CopMove16(cp, bplcon1, (i & 1) ? 0x0021 : 0x0010);
    CopMove32(cp, bplpt[0], screen->planes[0] + (i / 2) * 40);
    CopMove32(cp, bplpt[1], screen->planes[0] + (i / 2) * 40);
    CopMove32(cp, bplpt[2], screen->planes[2] + (i / 2) * 40);
    CopMove32(cp, bplpt[3], screen->planes[2] + (i / 2) * 40);
    CopMove32(cp, bplpt[4], screen->planes[4]);
  }
  CopEnd(cp);

  Print("Copper list entries: %ld\n", (LONG)(cp->curr - cp->entry));
}

void Kill() {
  DeletePalette(chunky->palette);
  DeletePixmap(chunky);
  DeleteBitmap(screen);
  DeleteCopList(cp);
}

void ChunkyToPlanar() {
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
  custom->bltafwm = -1;
  custom->bltalwm = -1;
  custom->bltsizv = 80 * 128 / 4;
  custom->bltsizh = 1;

  WaitBlitter();

  /* Swap 4x2, pass 2. */
  custom->bltapt = chunky->pixels + 80 * 128;
  custom->bltbpt = chunky->pixels + 80 * 128 + 2;
  custom->bltdpt = screen->planes[2] + 80 * 128 / 2;
  custom->bltamod = 2;
  custom->bltbmod = 2;
  custom->bltdmod = 0;
  custom->bltcdat = 0xf0f0;

  custom->bltcon0 = (SRCA | SRCB | DEST) | (ABC | ABNC | ANBC | NABNC) | (4 << ASHIFTSHIFT);
  custom->bltcon1 = BLITREVERSE;
  custom->bltafwm = -1;
  custom->bltalwm = -1;
  custom->bltsizv = 80 * 128 / 4;
  custom->bltsizh = 1;

  WaitBlitter();
}

void Main() {
  CopListActivate(cp);

  custom->dmacon = DMAF_SETCLR | DMAF_RASTER | DMAF_BLITTER;

  ChunkyToPlanar();

  WaitMouse();
}
