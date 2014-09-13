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

static BitmapT *screen[2];
static UWORD active = 0;
static CopInsT *bplptr[5];

static BitmapT *carry;
static BitmapT *flares;
static CopListT *cp;

void Load() {
  flares = LoadILBM("data/plotter-flares.ilbm", FALSE);
  screen[0] = NewBitmap(WIDTH, HEIGHT, 3, FALSE);
  screen[1] = NewBitmap(WIDTH, HEIGHT, 3, FALSE);
  carry = NewBitmap(SIZE + 16, SIZE, 2, FALSE);

  cp = NewCopList(100);
  CopInit(cp);
  CopMakePlayfield(cp, bplptr, screen[active]);
  CopMakeDispWin(cp, 0x81, 0x2c, WIDTH, HEIGHT);
  CopLoadPal(cp, flares->palette, 0);
  CopEnd(cp);
}

void Kill() {
  DeleteCopList(cp);
  DeletePalette(flares->palette);
  DeleteBitmap(flares);
  DeleteBitmap(carry);
  DeleteBitmap(screen[0]);
  DeleteBitmap(screen[1]);
}

static volatile LONG swapScreen = -1;
static volatile LONG frameCount = 0;

__interrupt_handler void IntLevel3Handler() {
  if (custom->intreqr & INTF_VERTB) {
    if (swapScreen >= 0) {
      BitmapT *buffer = screen[swapScreen];
      WORD n = buffer->depth;

      while (--n >= 0) {
        CopInsSet32(bplptr[n], buffer->planes[n]);
        custom->bplpt[n] = buffer->planes[n];
      }

      swapScreen = -1;
    }

    frameCount++;
  }

  custom->intreq = INTF_LEVEL3;
  custom->intreq = INTF_LEVEL3;
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

void Init() {
  InterruptVector->IntLevel3 = IntLevel3Handler;
  custom->intena = INTF_SETCLR | INTF_VERTB;
  
  CopListActivate(cp);
  custom->dmacon = DMAF_SETCLR | DMAF_RASTER | DMAF_BLITTER | DMAF_BLITHOG;
}

void Main() {
  while (!LeftMouseButton()) {
    ITER(i, 0, 2, BlitterSetSync(screen[active], i, 0, 0, 96 * 2 + SIZE, 96 * 2 + SIZE, 0));
    DrawPlotter();

    WaitVBlank();
    swapScreen = active;
    active ^= 1;
  }
}
