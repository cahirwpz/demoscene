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
static CopInsT *colors[2][HEIGHT];
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
  WORD i, j, k;

  CopInit(cp);
  CopMakePlayfield(cp, bplptr[n], twister, DEPTH);
  CopMakeDispWin(cp, X(STARTX), Y(0), WIDTH, HEIGHT);
  CopSetRGB(cp, 0, 0);

  colors[n][0] = CopWait(cp, Y(-1), X(STARTX + WIDTH));
  for (j = 1; j < 32; j++)
    CopSetRGB(cp, j, *pixels++);

  for (i = 0, k = 1; i < HEIGHT - 1; i++) {
    CopWait(cp, Y(i), 0);
    bplmod[n][i] = CopMove16(cp, bpl1mod, -32);
    CopMove16(cp, bpl2mod, -32);

    if ((i % 3) == 2) {
      colors[n][k++] = CopWait(cp, Y(i), X(STARTX + WIDTH));
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

static __regargs void SetupTexture(CopInsT **colors, WORD y) {
  WORD *pixels = texture->pixels;
  WORD height = texture->height;
  WORD width = texture->width;
  WORD n = height;

  y %= height;

  pixels += y * width;

  while (--n >= 0) {
    WORD *ins = (WORD *)(*colors++ + 1) + 1;

    ins[0] = *pixels++;
    ins[2] = *pixels++;
    ins[4] = *pixels++;
    ins[6] = *pixels++;
    ins[8] = *pixels++;
    ins[10] = *pixels++;
    ins[12] = *pixels++;
    ins[14] = *pixels++;
    ins[16] = *pixels++;
    ins[18] = *pixels++;
    ins[20] = *pixels++;
    ins[22] = *pixels++;
    ins[24] = *pixels++;
    ins[26] = *pixels++;
    ins[28] = *pixels++;
    ins[30] = *pixels++;
    ins[32] = *pixels++;
    ins[34] = *pixels++;
    ins[36] = *pixels++;
    ins[38] = *pixels++;
    ins[40] = *pixels++;
    ins[42] = *pixels++;
    ins[44] = *pixels++;
    ins[46] = *pixels++;
    ins[48] = *pixels++;
    ins[50] = *pixels++;
    ins[52] = *pixels++;
    ins[54] = *pixels++;
    ins[56] = *pixels++;
    ins[58] = *pixels++;
    ins[60] = *pixels++;

    y++;

    if (y >= height) {
      pixels = texture->pixels;
      y = 0;
    }
  }
}

static void Render() {
  // LONG lines = ReadLineCounter();
  SetupLines(frameCount * 16);
  SetupTexture(colors[active], frameCount);
  // Log("twister: %ld\n", ReadLineCounter() - lines);

  WaitVBlank();
  CopListActivate(cp[active]);
  active ^= 1;
}

EffectT Effect = { Load, UnLoad, Init, Kill, Render };
