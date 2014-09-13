#include "blitter.h"
#include "coplist.h"
#include "memory.h"
#include "tga.h"
#include "print.h"
#include "file.h"
#include "interrupts.h"

#define WIDTH 160
#define HEIGHT 100
#define X(x) ((x) + 0x81)
#define Y(y) ((y) + 0x2c + 28)

static PixmapT *chunky[2];
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

  cp = NewCopList(1024);
  screen[0] = NewBitmap(WIDTH * 2, HEIGHT * 2, 5, FALSE);
  screen[1] = NewBitmap(WIDTH * 2, HEIGHT * 2, 4, FALSE);

  memset(screen[0]->planes[4], 0xaa, WIDTH * HEIGHT * 4 / 8);

  {
    PixmapT *texture = LoadTGA("data/texture-16.tga", PM_CMAP, MEMF_PUBLIC);
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

  chunky[0] = NewPixmap(WIDTH, HEIGHT, PM_GRAY4, MEMF_CHIP);
  chunky[1] = NewPixmap(WIDTH, HEIGHT, PM_GRAY4, MEMF_CHIP);

  {
    UWORD *uvmap = ReadFile("data/uvmap.bin", MEMF_PUBLIC);
    UWORD *data = uvmap;
    UWORD *code = UVMapRenderTemplate;
    WORD n = WIDTH * HEIGHT / 2;

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
  CopMakeDispWin(cp, X(0), Y(0), screen[0]->width, screen[0]->height);
  CopLoadPal(cp, palette, 0);
  for (i = 16; i < 32; i++)
    CopSetRGB(cp, i, 0x000);
  for (i = 0; i < HEIGHT * 2; i++) {
    CopWaitMask(cp, Y(i), 0, 0xff, 0);
    CopMove16(cp, bplcon1, (i & 1) ? 0x0021 : 0x0010);
    CopMove16(cp, bpl1mod, (i & 1) ? -40 : 0);
    CopMove16(cp, bpl2mod, (i & 1) ? -40 : 0);
  }
  CopEnd(cp);
}

void Kill() {
  DeletePixmap(textureHi);
  DeletePixmap(textureLo);
  DeletePixmap(chunky[0]);
  DeletePixmap(chunky[1]);
  DeleteBitmap(screen[0]);
  DeleteBitmap(screen[1]);
  DeletePalette(palette);
  DeleteCopList(cp);
}

static struct {
  ULONG phase;
  BitmapT *screen;
  PixmapT *chunky;
} c2p = { 3, NULL, NULL };

static void InitChunkyToPlanar() {
  custom->bltamod = 2;
  custom->bltbmod = 2;
  custom->bltdmod = 0;
  custom->bltcdat = 0xf0f0;
  custom->bltafwm = -1;
  custom->bltalwm = -1;
}

static void ChunkyToPlanar() {
  BitmapT *screen = c2p.screen;
  PixmapT *chunky = c2p.chunky;

  switch (c2p.phase) {
    case 0:
      /* Swap 4x2, pass 1. */
      custom->bltapt = chunky->pixels;
      custom->bltbpt = chunky->pixels + 2;
      custom->bltdpt = screen->planes[0];

      custom->bltcon0 = (SRCA | SRCB | DEST) | (ABC | ABNC | ANBC | NABNC);
      custom->bltcon1 = 4 << BSHIFTSHIFT;
      custom->bltsize = 1;
      break;

    case 1:
      custom->bltsize = 1 | (976 << 6);
      break;

    case 2:
      /* Swap 4x2, pass 2. */
      // custom->bltapt = chunky->pixels + WIDTH * HEIGHT / 2;
      // custom->bltbpt = chunky->pixels + WIDTH * HEIGHT / 2 + 2;
      custom->bltdpt = screen->planes[2] + WIDTH * HEIGHT / 4;

      custom->bltcon0 = (SRCA | SRCB | DEST) | (ABC | ABNC | ANBC | NABNC) | (4 << ASHIFTSHIFT);
      custom->bltcon1 = BLITREVERSE;
      custom->bltsize = 1;
      break;

    case 3:
      custom->bltsize = 1 | (977 << 6);
      break;

    case 4:
      CopInsSet32(bpls[0], screen->planes[0]);
      CopInsSet32(bpls[1], screen->planes[0]);
      CopInsSet32(bpls[2], screen->planes[2]);
      CopInsSet32(bpls[3], screen->planes[2]);
      break;

    default:
      return;
  }

  c2p.phase++;
}

__interrupt_handler void IntLevel3Handler() {
  if (custom->intreqr & INTF_BLIT) {
    asm volatile("" ::: "d0", "d1", "a0", "a1");
    ChunkyToPlanar();
  }

  custom->intreq = INTF_LEVEL3;
  custom->intreq = INTF_LEVEL3;
}

void Init() {
  InterruptVector->IntLevel3 = IntLevel3Handler;
  custom->intena = INTF_SETCLR | INTF_VERTB | INTF_BLIT;

  CopListActivate(cp);
  custom->dmacon = DMAF_SETCLR | DMAF_RASTER | DMAF_BLITTER;
}

void Main() {
  InitChunkyToPlanar();

  while (!LeftMouseButton()) {
    UWORD offset = ReadFrameCounter();

    UBYTE *txtHi = textureHi->pixels + (offset & 16383);
    UBYTE *txtLo = textureLo->pixels + (offset & 16383);

#if 0
    {
      LONG lines = ReadLineCounter();

      (*UVMapRender)(chunky[active]->pixels, txtHi, txtLo);

      Log("uvmap: %ld\n", ReadLineCounter() - lines);
    }
#else
    (*UVMapRender)(chunky[active]->pixels, txtHi, txtLo);
#endif

    c2p.phase = 0;
    c2p.screen = screen[active];
    c2p.chunky = chunky[active];
    ChunkyToPlanar();

    active ^= 1;
  }
}
