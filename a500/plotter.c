#include "startup.h"
#include "hardware.h"
#include "coplist.h"
#include "gfx.h"
#include "ilbm.h"
#include "bltop.h"
#include "circle.h"
#include "fx.h"
#include "sync.h"
#include "memory.h"

#define WIDTH 320
#define HEIGHT 256
#define DEPTH 3
#define SIZE 16
#define NUM 37
#define ARMS 3

#define MAX_W 96
#define MAX_H 96

static CopListT *cp;
static CopInsT *bplptr[DEPTH];
static BitmapT *screen[2];
static UWORD active = 0;

static BitmapT *carry;
static BitmapT *flares;
static BitmapT *flare[8];

static void Load() {
  flares = LoadILBM("data/plotter-flares.ilbm");
}

static void UnLoad() {
  DeletePalette(flares->palette);
  DeleteBitmap(flares);
}

static void Init() {
  WORD i;

  screen[0] = NewBitmap(WIDTH, HEIGHT, DEPTH);
  screen[1] = NewBitmap(WIDTH, HEIGHT, DEPTH);

  custom->dmacon = DMAF_SETCLR | DMAF_BLITTER;

  for (i = 0; i < 8; i++) {
    flare[i] = NewBitmap(SIZE, SIZE, DEPTH);
    BitmapCopyArea(flare[i], 0, 0,
                   flares, 0, i * SIZE, SIZE, SIZE);
  }

  carry = NewBitmap(SIZE + 16, SIZE, 2);
  cp = NewCopList(50);

  for (i = 0; i < 2; i++)
    BitmapClear(screen[i], DEPTH);

  CopInit(cp);
  CopMakeDispWin(cp, X(0), Y(0), WIDTH, HEIGHT);
  CopMakePlayfield(cp, bplptr, screen[active], DEPTH);
  CopLoadPal(cp, flares->palette, 0);
  CopEnd(cp);
  CopListActivate(cp);
  custom->dmacon = DMAF_SETCLR | DMAF_RASTER;
}

static void Kill() {
  custom->dmacon = DMAF_COPPER | DMAF_BLITTER | DMAF_RASTER;

  ITER(i, 0, 7, DeleteBitmap(flare[i]));
  DeleteBitmap(carry);
  DeleteBitmap(screen[0]);
  DeleteBitmap(screen[1]);
  DeleteCopList(cp);
}

static void DrawPlotter() {
  WORD i, a;
  WORD t = frameCount * 5;
  WORD da = 2 * SIN_PI / NUM;

  for (i = 0, a = t; i < NUM; i++, a += da) {
    WORD g = SIN(ARMS * a);
    WORD x = normfx(normfx(SIN(t + a) * g) * 96) + 96;
    WORD y = normfx(normfx(COS(t + a) * g) * 96) + 96;
    WORD f = normfx(g * 3);

    if (f < 0)
      f = -f;

    if ((i & 1) && (frameCount & 15) < 3)
      f = 7;

    BitmapAddSaturated(screen[active], x, y, flare[f], carry);
  }
}

static void Render() {
  BitmapClearArea(screen[active], DEPTH, 0, 0,
                  MAX_W * 2 + SIZE, MAX_H * 2 + SIZE);
  DrawPlotter();

  WaitVBlank();
  ITER(i, 0, DEPTH - 1, CopInsSet32(bplptr[i], screen[active]->planes[i]));
  active ^= 1;
}

EffectT Effect = { Load, UnLoad, Init, Kill, Render };
