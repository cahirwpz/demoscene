#include "startup.h"
#include "blitter.h"
#include "coplist.h"
#include "interrupts.h"
#include "memory.h"
#include "ilbm.h"
#include "2d.h"
#include "fx.h"
#include "circle.h"

#define WIDTH 320
#define HEIGHT 256
#define SIZE 128

static BitmapT *screen[2];
static UWORD active = 0;

static BitmapT *clip;
static BitmapT *carry;
static BitmapT *buffer;
static PaletteT *palette[2];
static CopInsT *bplptr[2][5];
static CopListT *cp;

static void Load() {
  clip = LoadILBM("data/blurred-clip.ilbm", FALSE);

  palette[0] = LoadPalette("data/blurred1.ilbm");
  palette[1] = LoadPalette("data/blurred2.ilbm");

  screen[0] = NewBitmap(WIDTH, HEIGHT, 5, FALSE);
  screen[1] = NewBitmap(WIDTH, HEIGHT, 5, FALSE);
  buffer = NewBitmap(SIZE, SIZE, 4, FALSE);
  carry = NewBitmap(SIZE, SIZE, 2, FALSE);

  cp = NewCopList(200);
  CopInit(cp);
  CopLoadPal(cp, palette[0], 0);
  CopMakeDispWin(cp, X(0), Y(0), WIDTH, HEIGHT);
  CopMakePlayfield(cp, bplptr[0], screen[active]);
  CopWait(cp, Y(127), 0);
  CopLoadPal(cp, palette[1], 0);
  CopWait(cp, Y(128), 0);
  {
    WORD i;
    for (i = 0; i < screen[active]->depth; i++)
      bplptr[1][i] = CopMove32(cp, bplpt[i], screen[active]->planes[i] - WIDTH / 16);
  }
  CopEnd(cp);
}

static void UnLoad() {
  DeleteCopList(cp);
  DeletePalette(clip->palette);
  DeleteBitmap(clip);
  DeleteBitmap(screen[0]);
  DeleteBitmap(screen[1]);
  DeleteBitmap(buffer);
  DeleteBitmap(carry);
  DeletePalette(palette[0]);
  DeletePalette(palette[1]);
}

static volatile LONG swapScreen = -1;
static ULONG iterCount = 0;

static __interrupt_handler void IntLevel3Handler() {
  if (custom->intreqr & INTF_VERTB) {
    if (swapScreen >= 0) {
      BitmapT *buffer = screen[swapScreen];
      WORD n = 4;

      while (--n >= 0) {
        CopInsSet32(bplptr[0][n], buffer->planes[n]);
        CopInsSet32(bplptr[1][n], buffer->planes[n] - WIDTH / 16);
        custom->bplpt[n] = buffer->planes[n];
      }

      swapScreen = -1;
    }
  }

  custom->intreq = INTF_LEVEL3;
  custom->intreq = INTF_LEVEL3;
}

static void RotatingTriangle(WORD t, WORD phi, WORD size) {
  Point2D p[3];
  WORD i, j;

  /* Calculate vertices of a rotating triangle. */
  for (i = 0; i < 3; i++) {
    WORD k = SIN(t + phi) / 2;
    WORD x = SIN(k + i * (SIN_PI * 2 / 3));
    WORD y = COS(k + i * (SIN_PI * 2 / 3));

    p[i].x = normfx((WORD)size * (WORD)x) / 2 + SIZE / 2;
    p[i].y = normfx((WORD)size * (WORD)y) / 2 + SIZE / 2;
  }

  /* Create a bob with rotating triangle. */
  for (i = 0, j = 1; i < 3; i++, j = (i == 2 ? 0 : i + 1))
    BlitterLineSync(p[i].x, p[i].y, p[j].x, p[j].y);
}

static void DrawShape() {
  WaitBlitter();
  BlitterClear(carry, 0);

  WaitBlitter();
  BlitterLineSetup(carry, 0, LINE_EOR, LINE_ONEDOT);

  RotatingTriangle(iterCount * 16, 0, SIZE);
  RotatingTriangle(iterCount * 16, SIN_PI * 2 / 3, SIZE);
  RotatingTriangle(-iterCount * 16, SIN_PI * 2 / 3, SIZE / 2);

  WaitBlitter();
  BlitterFill(carry, 0);
}

#define BLTOP_NAME DecrementAndSaturate
#define BLTOP_BORROW_BM carry
#define BLTOP_DST_BM buffer
#define BLTOP_HSIZE SIZE
#define BLTOP_VSIZE SIZE
#define BLTOP_BPLS 4
#include "bltop_dec_sat.h"

#define BLTOP_NAME IncrementAndSaturate
#define BLTOP_CARRY_BM carry
#define BLTOP_DST_BM buffer
#define BLTOP_HSIZE SIZE
#define BLTOP_VSIZE SIZE
#define BLTOP_BPLS 4
#include "bltop_inc_sat.h"

static void Init() {
  InterruptVector->IntLevel3 = IntLevel3Handler;
  custom->intena = INTF_SETCLR | INTF_VERTB;
  
  CopListActivate(cp);
  custom->dmacon = DMAF_SETCLR | DMAF_BLITTER | DMAF_RASTER | DMAF_BLITHOG;

  /* Make the center of blurred shape use colors from range 16-31. */
  CircleEdge(screen[0], 4, SIZE / 2 + 16, SIZE / 2, SIZE / 4 - 1);
  BlitterFill(screen[0], 4);
  WaitBlitter();

  ITER(i, 0, 4, BlitterCopySync(screen[0], i, WIDTH / 2, 0, clip, i));
  ITER(i, 0, 4, BlitterCopySync(screen[1], i, WIDTH / 2, 0, clip, i));
}

static void Render() {
  LONG lines = ReadLineCounter();

  if (iterCount++ & 1)
    DecrementAndSaturate();
  DrawShape();
  IncrementAndSaturate();

  ITER(i, 0, 3, BlitterCopySync(screen[active], i, 16, 0, buffer, i));

  Log("loop: %ld\n", ReadLineCounter() - lines);

  swapScreen = active;
  active ^= 1;
}

EffectT Effect = { Load, UnLoad, Init, NULL, Render };
