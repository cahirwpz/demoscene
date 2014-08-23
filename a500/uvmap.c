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
static CopListT *cp;
static CopInsT *bpls[6];

extern WORD UVMapRenderTemplate[];
void (*UVMapRender)(UBYTE *chunky asm("a0"),
                    UBYTE *textureHi asm("a1"),
                    UBYTE *textureLo asm("a2"));

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

  {
    UWORD *uvmap = ReadFile("data/uvmap.bin", MEMF_PUBLIC);
    UWORD *data = uvmap;
    UWORD *code = UVMapRenderTemplate;
    WORD n = WIDTH * HEIGHT / 4 / 8;

    while (n--) {
      /* scramble: [ab] [cd] [ef] [gh] => [ab] [ef] [cd] [gh] */
      code[1] = *data++;
      code[3] = *data++;
      code[11] = *data++;
      code[13] = *data++;
      code[6] = *data++;
      code[8] = *data++;
      code[16] = *data++;
      code[18] = *data++;
      code += 20;
    }

    UVMapRender = (void *)UVMapRenderTemplate;

    FreeAutoMem(uvmap);
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

void Main() {
  InterruptVector->IntLevel3 = IntLevel3Handler;
  custom->intena = INTF_SETCLR | INTF_VERTB;

  CopListActivate(cp);
  custom->dmacon = DMAF_SETCLR | DMAF_RASTER | DMAF_BLITTER;

  while (!LeftMouseButton()) {
    UBYTE *txtHi = textureHi->pixels + 32768 + (frameCount & 255);
    UBYTE *txtLo = textureLo->pixels + 32768 + (frameCount & 255);

    {
      LONG lines = ReadLineCounter();

      (*UVMapRender)(chunky->pixels, txtHi, txtLo);

      Log("uvmap: %ld\n", ReadLineCounter() - lines);
    }

    {
      LONG lines = ReadLineCounter();

      ChunkyToPlanar();

      Log("c2p: %ld\n", ReadLineCounter() - lines);
    }
  }
}
