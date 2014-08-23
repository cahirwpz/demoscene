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
static BitmapT *screen[2];
static PaletteT *palette;
static UWORD active = 0;
static CopListT *cp;
static CopInsT *bpls[6];

extern WORD UVMapRenderTemplate[];
void (*UVMapRender)(UBYTE *chunky asm("a0"),
                    UBYTE *textureHi asm("a1"),
                    UBYTE *textureLo asm("a2"));

static void PixmapScramble(PixmapT *image, PixmapT *imageHi, PixmapT *imageLo)
{
  UBYTE *data = image->pixels;
  UBYTE *hi = imageHi->pixels;
  UBYTE *lo = imageLo->pixels;
  LONG n = image->width * image->height;

  do {
    BYTE c = *data++;
    /* [0 0 0 0 a0 a1 a2 a3] => [a2 a3 0 0 a0 a1 0 0] */
    *hi++ = (c & 0x0c) | ((c & 0x03) << 6);
    /* [0 0 0 0 a0 a1 a2 a3] => [ 0 0 a2 a3 0 0 a0 a1] */
    *lo++ = ((c & 0x0c) >> 2) | ((c & 0x03) << 4);
  } while (--n);
}

void Load() {
  UWORD i;

  cp = NewCopList(4096);
  screen[0] = NewBitmap(WIDTH, HEIGHT, 5, FALSE);
  screen[1] = NewBitmap(WIDTH, HEIGHT, 4, FALSE);

  memset(screen[0]->planes[4], 0xaa, WIDTH * HEIGHT / 8);

  {
    PixmapT *texture = LoadTGA("data/texture-16.tga", PM_CMAP);
    LONG size = texture->width * texture->height;

    palette = texture->palette;

    textureHi = NewPixmap(texture->width, texture->height * 2,
                          PM_CMAP, MEMF_PUBLIC|MEMF_CLEAR);
    textureLo = NewPixmap(texture->width, texture->height * 2,
                          PM_CMAP, MEMF_PUBLIC|MEMF_CLEAR);
    PixmapScramble(texture, textureHi, textureLo);

    /* Extra halves for cheap texture motion. */
    memcpy(textureHi->pixels + size, textureHi->pixels, size);
    memcpy(textureLo->pixels + size, textureLo->pixels, size);

    DeletePixmap(texture);
  }

  chunky = NewPixmap(WIDTH / 2, HEIGHT / 2, PM_GRAY4, MEMF_CHIP);

  {
    UWORD *uvmap = ReadFile("data/uvmap.bin", MEMF_PUBLIC);
    UWORD *data = uvmap;
    UWORD *code = UVMapRenderTemplate;
    WORD n = WIDTH * HEIGHT / 8;

    /* UVMap is pre-scrambled. */
    while (n--) {
      code++;
      *code++ = *data++;
      code++;
      *code++ = *data++;
      code++;
    }

    UVMapRender = (void *)UVMapRenderTemplate;

    FreeAutoMem(uvmap);
  }

  CopInit(cp);
  CopMakePlayfield(cp, bpls, screen[0]);
  CopMakeDispWin(cp, 0x81, 0x2c, screen[0]->width, screen[0]->height);
  CopLoadPal(cp, palette, 0);
  for (i = 16; i < 32; i++)
    CopSetRGB(cp, i, 0x000);
  for (i = 0; i < 256; i++) {
    CopWaitMask(cp, 0x2c + i, 0, 0xff, 0);
    CopMove16(cp, bplcon1, (i & 1) ? 0x0021 : 0x0010);
    CopMove16(cp, bpl1mod, (i & 1) ? -40 : 0);
    CopMove16(cp, bpl2mod, (i & 1) ? -40 : 0);
  }
  CopEnd(cp);
}

void Kill() {
  DeletePixmap(textureHi);
  DeletePixmap(textureLo);
  DeletePixmap(chunky);
  DeletePalette(palette);
  DeleteBitmap(screen[0]);
  DeleteBitmap(screen[1]);
  DeleteCopList(cp);
}

void ChunkyToPlanar(BitmapT *screen) {
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
    UWORD offset = frameCount;

    UBYTE *txtHi = textureHi->pixels + (offset & 16383);
    UBYTE *txtLo = textureLo->pixels + (offset & 16383);

#ifdef PROFILING
    {
      LONG lines = ReadLineCounter();

      (*UVMapRender)(chunky->pixels, txtHi, txtLo);

      Log("uvmap: %ld\n", ReadLineCounter() - lines);
    }
#else
    (*UVMapRender)(chunky->pixels, txtHi, txtLo);
#endif

#ifdef PROFILING
    {
      LONG lines = ReadLineCounter();

      ChunkyToPlanar(screen[active]);

      Log("c2p: %ld\n", ReadLineCounter() - lines);
    }
#else
    ChunkyToPlanar(screen[active]);
#endif

    CopInsSet32(bpls[0], screen[active]->planes[0]);
    CopInsSet32(bpls[1], screen[active]->planes[0]);
    CopInsSet32(bpls[2], screen[active]->planes[2]);
    CopInsSet32(bpls[3], screen[active]->planes[2]);

    active ^= 1;
  }
}
