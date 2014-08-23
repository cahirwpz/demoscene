#include "blitter.h"
#include "coplist.h"
#include "memory.h"
#include "tga.h"
#include "print.h"
#include "file.h"
#include "interrupts.h"

#define WIDTH 320
#define HEIGHT 256

static PixmapT *chunky;
static PixmapT *textureHi, *textureLo;
static BitmapT *screen;
static UWORD *uvmap;
static CopListT *cp;
static CopInsT *bpls[6];

static void PixmapScrambleHi(PixmapT *image) {
  UBYTE *data = image->pixels;
  LONG n = image->width * image->height;

  /* [0 0 0 0 a0 a1 a2 a3] => [a2 a3 0 0 a0 a1 0 0] */
  do {
    BYTE c = *data;
    *data++ = (c & 0x0c) | ((c & 0x03) << 6);
  } while (--n);
}

static void PixmapScrambleLo(PixmapT *image) {
  UBYTE *data = image->pixels;
  LONG n = image->width * image->height;

  /* [0 0 0 0 a0 a1 a2 a3] => [ 0 0 a2 a3 0 0 a0 a1] */
  do {
    BYTE c = *data;
    *data++ = ((c & 0x0c) >> 2) | ((c & 0x03) << 4);
  } while (--n);
}

void Load() {
  UWORD i;

  cp = NewCopList(4096);
  screen = NewBitmap(WIDTH, HEIGHT, 5, FALSE);
  memset(screen->planes[4], 0xaa, WIDTH * HEIGHT / 8);

  textureHi = LoadTGA("data/texture-16.tga", PM_CMAP);
  textureLo = CopyPixmap(textureHi);
  PixmapScrambleHi(textureHi);
  PixmapScrambleLo(textureLo);

  chunky = NewPixmap(WIDTH / 2, HEIGHT / 2, PM_GRAY4, MEMF_CHIP|MEMF_CLEAR);
  chunky->palette = textureHi->palette;

  uvmap = ReadFile("data/uvmap.bin", MEMF_PUBLIC);

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
  FreeAutoMem(uvmap);
  DeletePixmap(textureHi);
  DeletePixmap(textureLo);
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

static ULONG frameCount = 0;

__interrupt_handler void IntLevel3Handler() {
  if (custom->intreqr & INTF_VERTB)
    frameCount++;

  custom->intreq = INTF_LEVEL3;
  custom->intreq = INTF_LEVEL3;
}

static inline UBYTE load(UBYTE *hi, UBYTE *lo, UWORD o1, UWORD o2) {
  UBYTE res;

  asm("moveb %2@(%3:w),%0\n"
      "orb   %1@(%4:w),%0\n"
      : "=d" (res)
      : "a" (lo), "a" (hi), "d" (o1), "d" (o2));

  return res;
}

void Main() {
  InterruptVector->IntLevel3 = IntLevel3Handler;
  custom->intena = INTF_SETCLR | INTF_VERTB;

  CopListActivate(cp);
  custom->dmacon = DMAF_SETCLR | DMAF_RASTER | DMAF_BLITTER;

  while (!LeftMouseButton()) {
    register UBYTE *txtHi = textureHi->pixels + 32768 + (frameCount & 255);
    register UBYTE *txtLo = textureLo->pixels + 32768 + (frameCount & 255);
    register UBYTE *chk = chunky->pixels;
    register UWORD *uv = uvmap;
    LONG n = (HEIGHT / 2) * (WIDTH / 2) / 8;

    while (n--) {
      register UWORD d0 asm("d0");
      register UWORD d1 asm("d1");
      register UWORD d2 asm("d2");
      register UWORD d3 asm("d3");
      register UWORD d4 asm("d4");
      register UWORD d5 asm("d5");
      register UWORD d6 asm("d6");
      register UWORD d7 asm("d7");

      asm("movemw %0@+,%1/%2/%3/%4/%5/%6/%7/%8"
          : "+a" (uv),
            "=d" (d0), "=d" (d1), "=d" (d2), "=d" (d3),
            "=d" (d4), "=d" (d5), "=d" (d6), "=d" (d7));

      {
        UBYTE a = load(txtHi, txtLo, d0, d1);
        UBYTE b = load(txtHi, txtLo, d2, d3);
        UBYTE c = load(txtHi, txtLo, d4, d5);
        UBYTE d = load(txtHi, txtLo, d6, d7);

        *chk++ = a;
        *chk++ = c;
        *chk++ = b;
        *chk++ = d;
      }
    }

    ChunkyToPlanar();
  }
}
