#include "startup.h"
#include "blitter.h"
#include "coplist.h"
#include "ilbm.h"
#include "tga.h"
#include "fx.h"
#include "memory.h"
#include "random.h"

#define WIDTH   128
#define HEIGHT  255
#define DEPTH   5
#define STARTX  96

static BitmapT *twister;
static PixmapT *texture;
static CopListT *cp[2];
static CopInsT *bplptr[2][DEPTH];
static CopInsT *bplmod[2][HEIGHT - 1];
static WORD active = 0;

static void Load() {
  twister = LoadILBM("data/twister.ilbm");
  texture = LoadTGA("data/twister-texture.tga", PM_RGB4, MEMF_PUBLIC);
}

static void UnLoad() {
  DeletePalette(twister->palette);
  DeleteBitmap(twister);
  DeletePixmap(texture);
}

static void MakeCopperList(CopListT **ptr, WORD n) {
  CopListT *cp = NewCopList(100 + HEIGHT * 3 + (32 * HEIGHT / 3));
  WORD *pixels = texture->pixels;
  WORD i, j;

  CopInit(cp);
  CopMakePlayfield(cp, bplptr[n], twister, DEPTH);
  CopMakeDispWin(cp, X(STARTX), Y(0), WIDTH, HEIGHT);
  CopMove16(cp, bplcon0, 0x5200);
  CopSetRGB(cp, 0, 0);
  for (j = 1; j < 32; j++)
    CopSetRGB(cp, j, *pixels++);
  for (i = 0; i < HEIGHT - 1; i++) {
    CopWait(cp, Y(i), 0);
    bplmod[n][i] = CopMove16(cp, bpl1mod, -32);
    CopMove16(cp, bpl2mod, -32);

    if ((i % 3) == 2) {
      CopWait(cp, Y(i), X(STARTX + WIDTH));
      for (j = 1; j < 32; j++)
        CopSetRGB(cp, j, *pixels++);
    }
  }
  CopEnd(cp);

  *ptr = cp;
}

static void Init() {
  custom->dmacon = DMAF_SETCLR | DMAF_BLITTER;

  MakeCopperList(&cp[0], 0);
  MakeCopperList(&cp[1], 1);

  CopListActivate(cp[1]);
  custom->dmacon = DMAF_SETCLR | DMAF_RASTER;
}

static void Kill() {
  DeleteCopList(cp[0]);
  DeleteCopList(cp[1]);
}

static inline WORD rotate(WORD f) {
  return (COS(f) >> 3) & 255;
}

static void SetupLines(WORD f) {
  WORD y0 = rotate(f);

  /* first line */
  {
    LONG y = (WORD)twister->bytesPerRow * y0;
    APTR *planes = twister->planes;
    CopInsT **bpl = bplptr[active];
    WORD n = DEPTH;

    while (--n >= 0)
      CopInsSet32(*bpl++, (*planes++) + y);
  }

  /* consecutive lines */
  {
    WORD **modptr = (WORD **)bplmod[active];
    WORD y1, i, m;

    for (i = 1; i < HEIGHT; i++, y0 = y1, f += 2) {
      WORD *mod = *modptr++;

      y1 = rotate(f);
      m = ((y1 - y0) - 1) * (WIDTH / 8);

      mod[1] = m;
      mod[3] = m;
    }
  }
}

static void Render() {
  // LONG lines = ReadLineCounter();
  SetupLines(frameCount * 16);
  // Log("twister: %ld\n", ReadLineCounter() - lines);

  WaitVBlank();
  CopListActivate(cp[active]);
  active ^= 1;
}

EffectT Effect = { Load, UnLoad, Init, Kill, Render };
