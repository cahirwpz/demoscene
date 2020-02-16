#include "startup.h"
#include "hardware.h"
#include "coplist.h"
#include "gfx.h"
#include "ilbm.h"
#include "blitter.h"
#include "circle.h"
#include "fx.h"
#include "sync.h"
#include "memory.h"
#include "tasks.h"

const char *__cwdpath = "data";

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
static u_short active = 0;

static BitmapT *carry;
static BitmapT *flares;
static BitmapT *flare[8];

static void Load(void) {
  flares = LoadILBM("plotter-flares.ilbm");
}

static void UnLoad(void) {
  DeletePalette(flares->palette);
  DeleteBitmap(flares);
}

static void Init(void) {
  short i;

  screen[0] = NewBitmap(WIDTH, HEIGHT, DEPTH);
  screen[1] = NewBitmap(WIDTH, HEIGHT, DEPTH);

  EnableDMA(DMAF_BLITTER);

  for (i = 0; i < 8; i++) {
    Area2D flare_area = { 0, i * SIZE, SIZE, SIZE };
    flare[i] = NewBitmap(SIZE, SIZE, DEPTH);
    BitmapCopyArea(flare[i], 0, 0, flares, &flare_area);
  }

  carry = NewBitmap(SIZE + 16, SIZE, 2);
  cp = NewCopList(50);

  for (i = 0; i < 2; i++)
    BitmapClear(screen[i]);

  CopInit(cp);
  CopSetupGfxSimple(cp, MODE_LORES, DEPTH, X(0), Y(0), WIDTH, HEIGHT);
  CopWait(cp, Y(-1), 0);
  CopSetupBitplanes(cp, bplptr, screen[active], DEPTH);
  CopLoadPal(cp, flares->palette, 0);
  CopEnd(cp);
  CopListActivate(cp);
  EnableDMA(DMAF_RASTER);
}

static void Kill(void) {
  DisableDMA(DMAF_COPPER | DMAF_BLITTER | DMAF_RASTER);

  ITER(i, 0, 7, DeleteBitmap(flare[i]));
  DeleteBitmap(carry);
  DeleteBitmap(screen[0]);
  DeleteBitmap(screen[1]);
  DeleteCopList(cp);
}

static void DrawPlotter(void) {
  short i, a;
  short t = frameCount * 5;
  short da = 2 * SIN_PI / NUM;

  for (i = 0, a = t; i < NUM; i++, a += da) {
    short g = SIN(ARMS * a);
    short x = normfx(normfx(SIN(t + a) * g) * 96) + 96;
    short y = normfx(normfx(COS(t + a) * g) * 96) + 96;
    short f = normfx(g * 3);

    if (f < 0)
      f = -f;

    if ((i & 1) && (frameCount & 15) < 3)
      f = 7;

    BitmapAddSaturated(screen[active], x, y, flare[f], carry);
  }
}

static void Render(void) {
  BitmapClearArea(screen[active], 
                  STRUCT(Area2D, 0, 0, MAX_W * 2 + SIZE, MAX_H * 2 + SIZE));
  DrawPlotter();

  ITER(i, 0, DEPTH - 1, CopInsSet32(bplptr[i], screen[active]->planes[i]));
  TaskWait(VBlankEvent);
  active ^= 1;
}

EffectT Effect = { Load, UnLoad, Init, Kill, Render, NULL };
