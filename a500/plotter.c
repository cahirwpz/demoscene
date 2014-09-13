#include "hardware.h"
#include "coplist.h"
#include "gfx.h"
#include "ilbm.h"
#include "interrupts.h"
#include "blitter.h"
#include "circle.h"
#include "fx.h"

#define WIDTH 320
#define HEIGHT 256
#define SIZE 16
#define NUM 37
#define ARMS 3

static CopListT *cp[2];
static BitmapT *screen[2];
static UWORD active = 0;

static BitmapT *carry;
static BitmapT *flares;

static void MakeCopperList(CopListT *cp, UWORD num) {
  CopInit(cp);
  CopMakeDispWin(cp, 0x81, 0x2c, WIDTH, HEIGHT);
  CopShowPlayfield(cp, screen[num]);
  CopLoadPal(cp, flares->palette, 0);
  CopEnd(cp);
}

void Load() {
  flares = LoadILBM("data/plotter-flares.ilbm", FALSE);
  screen[0] = NewBitmap(WIDTH, HEIGHT, 3, FALSE);
  screen[1] = NewBitmap(WIDTH, HEIGHT, 3, FALSE);
  carry = NewBitmap(SIZE + 16, SIZE, 2, FALSE);

  cp[0] = NewCopList(50);
  MakeCopperList(cp[0], 0);
  cp[1] = NewCopList(50);
  MakeCopperList(cp[1], 1);
}

void Kill() {
  DeletePalette(flares->palette);
  DeleteBitmap(flares);
  DeleteBitmap(carry);
  DeleteBitmap(screen[0]);
  DeleteBitmap(screen[1]);
  DeleteCopList(cp[0]);
  DeleteCopList(cp[1]);
}

static LONG frameCount = 0;

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

void Init() {
  CopListActivate(cp[active]);
  custom->dmacon = DMAF_SETCLR | DMAF_RASTER | DMAF_BLITTER | DMAF_BLITHOG;
}

void Main() {
  while (!LeftMouseButton()) {
    frameCount = ReadFrameCounter();

    ITER(i, 0, 2, BlitterSetSync(screen[active], i, 0, 0, 96 * 2 + SIZE, 96 * 2 + SIZE, 0));
    DrawPlotter();

    CopListRun(cp[active]);
    WaitVBlank();
    active ^= 1;
  }
}
