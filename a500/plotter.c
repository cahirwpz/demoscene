#include "startup.h"
#include "hardware.h"
#include "coplist.h"
#include "gfx.h"
#include "ilbm.h"
#include "blitter.h"
#include "circle.h"
#include "fx.h"

#define WIDTH 320
#define HEIGHT 256
#define DEPTH 3
#define SIZE 16
#define NUM 37
#define ARMS 3

static CopListT *cp;
static CopInsT *bplptr[DEPTH];
static BitmapT *screen[2];
static UWORD active = 0;

static BitmapT *carry;
static BitmapT *flares;

static void Load() {
  flares = LoadILBM("data/plotter-flares.ilbm", FALSE);
  screen[0] = NewBitmap(WIDTH, HEIGHT, DEPTH, FALSE);
  screen[1] = NewBitmap(WIDTH, HEIGHT, DEPTH, FALSE);
  carry = NewBitmap(SIZE + 16, SIZE, 2, FALSE);
  cp = NewCopList(50);
}

static void UnLoad() {
  DeletePalette(flares->palette);
  DeleteBitmap(flares);
  DeleteBitmap(carry);
  DeleteBitmap(screen[0]);
  DeleteBitmap(screen[1]);
  DeleteCopList(cp);
}

static void MakeCopperList(CopListT *cp) {
  CopInit(cp);
  CopMakeDispWin(cp, X(0), Y(0), WIDTH, HEIGHT);
  CopMakePlayfield(cp, bplptr, screen[active]);
  CopLoadPal(cp, flares->palette, 0);
  CopEnd(cp);
}

static void Init() {
  MakeCopperList(cp);
  CopListActivate(cp);
  custom->dmacon = DMAF_SETCLR | DMAF_RASTER | DMAF_BLITTER | DMAF_BLITHOG;
}

#define BLTOP_NAME AddFlare
#define BLTOP_SRC_BM flares
#define BLTOP_SRC_WIDTH SIZE
#define BLTOP_CARRY_BM carry
#define BLTOP_DST_BM screen[active]
#define BLTOP_DST_WIDTH WIDTH
#define BLTOP_HSIZE SIZE
#define BLTOP_VSIZE SIZE
#define BLTOP_BPLS 3
#include "bltop_add_sat.h"

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

    AddFlare(x, y, 0, f * SIZE);
  }
}

static void Render() {
  ITER(i, 0, 2, BlitterSetSync(screen[active], i, 0, 0, 96 * 2 + SIZE, 96 * 2 + SIZE, 0));
  DrawPlotter();

  WaitVBlank();
  ITER(i, 0, DEPTH - 1, CopInsSet32(bplptr[i], screen[active]->planes[i]));
  active ^= 1;
}

EffectT Effect = { Load, UnLoad, Init, NULL, Render };
