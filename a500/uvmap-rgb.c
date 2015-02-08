#include "startup.h"
#include "blitter.h"
#include "coplist.h"
#include "memory.h"
#include "file.h"
#include "tga.h"

#define WIDTH 80
#define HEIGHT 128
#define DEPTH 1

static BitmapT *screen;
static CopListT *coplist[2];
static CopInsT *chunky[2];
static UWORD active = 0;
static UWORD *texture;
static UWORD *uvmap;

#define UVMapRenderSize (WIDTH * HEIGHT * 12 + 4)
static void (*UVMapRender)(CopInsT *chunky asm("a0"),
                           UWORD *texture asm("a1"));

static void MakeUVMapRenderCode() {
  UWORD *code = (APTR)UVMapRender;
  UWORD *data = uvmap;
  WORD j = HEIGHT * 2;
  LONG pos = 6 - 4096 * sizeof(CopInsT);

  while (--j >= 0) {
    WORD i = WIDTH / 2;

    while (--i >= 0) {
      /* 3169 aaaa bbbb => move.w aaaa(a1), bbbb(a0) */
      *code++ = 0x3169;
      *code++ = *data++;
      *code++ = pos;
      pos += 4;
    }

    pos += 4;
  }

  *code++ = 0x4e75; /* RTS */
}

static void Load() {
  uvmap = ReadFile("data/uvmap-rgb.bin", MEMF_PUBLIC);

  {
    PixmapT *image = LoadTGA("data/texture-rgb.tga", PM_RGB4, MEMF_PUBLIC);

    texture = MemAllocAuto(65536, MEMF_PUBLIC);

    memcpy(texture, image->pixels, 32768);
    memcpy(texture + 16384, image->pixels, 32768);

    DeletePixmap(image);
  }

  UVMapRender = MemAlloc(UVMapRenderSize, MEMF_PUBLIC);
  MakeUVMapRenderCode();
}

static void UnLoad() {
  MemFree(UVMapRender, UVMapRenderSize);
  MemFreeAuto(uvmap);
  MemFreeAuto(texture);
}

static CopInsT *MakeCopperList(CopListT *cp) {
  CopInsT *chunky;
  UWORD i, j;

  CopInit(cp);
  CopMakePlayfield(cp, NULL, screen, screen->depth);
  CopMakeDispWin(cp, X(3), Y(0), screen->width, screen->height);
  for (i = 0; i < 32; i++)
    CopSetRGB(cp, i, 0);
  chunky = cp->curr;
  for (j = 0; j < HEIGHT * 2; j++) {
    CopWait(cp, Y(j) & 255, (j & 1) ? 0x40 : 0x3e);
    for (i = 0; i < WIDTH / 2; i++)
      CopSetRGB(cp, 1, 0);
  }
  CopEnd(cp);

  return chunky;
}

static void Init() {
  screen = NewBitmap(WIDTH * 4, HEIGHT * 2, DEPTH);

  /*
   * Chunky buffer has one "unused" pixel at the beginning of each half-line.
   * Actually it's the last word of copper wait instruction.
   *
   * Moreover the buffer is scrambled. Even pixels are moved to even half-lines
   * and odd pixels to odd half-lines respectively.
   */

  {
    UWORD *plane = screen->planes[0];
    UWORD i, j;

    for (j = 0; j < HEIGHT * 2; j += 2) {
      for (i = 0; i < WIDTH * 4 / 16; i++)
        *plane++ = 0x5f5f;
      for (i = 0; i < WIDTH * 4 / 16; i++)
        *plane++ = 0xf5f5;
    }
  }

  coplist[0] = NewCopList((WIDTH + 2) * HEIGHT + 100);
  coplist[1] = NewCopList((WIDTH + 2) * HEIGHT + 100);

  chunky[0] = MakeCopperList(coplist[0]);
  chunky[1] = MakeCopperList(coplist[1]);

  CopListActivate(coplist[1]);
  custom->dmacon = DMAF_SETCLR | DMAF_RASTER;
}

static void Kill() {
  custom->dmacon = DMAF_COPPER | DMAF_RASTER;

  DeleteCopList(coplist[0]);
  DeleteCopList(coplist[1]);
  DeleteBitmap(screen);
}

static void Render() {
  // LONG lines = ReadLineCounter();
  (*UVMapRender)(chunky[active] + 4096, &texture[frameCount & 16383]);
  // Log("uvmap-rgb: %ld\n", ReadLineCounter() - lines);

  CopListActivate(coplist[active]);
  active ^= 1;
}

EffectT Effect = { Load, UnLoad, Init, Kill, Render };
