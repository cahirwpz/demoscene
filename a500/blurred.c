#include "blitter.h"
#include "coplist.h"
#include "interrupts.h"
#include "memory.h"
#include "ilbm.h"
#include "2d.h"
#include "fx.h"
#include "circle.h"

#define X(x) ((x) + 0x81)
#define Y(y) ((y) + 0x2c)

#define WIDTH 320
#define HEIGHT 256
#define SIZE 128

static BitmapT *screen[2];
static UWORD active = 0;

static BitmapT *carry;
static BitmapT *buffer;
static PaletteT *palette[2];
static CopInsT *bplptr[2][5];
static CopListT *cp;

void Load() {
  palette[0] = LoadPalette("data/blurred1.ilbm");
  palette[1] = LoadPalette("data/blurred2.ilbm");

  screen[0] = NewBitmap(WIDTH, HEIGHT, 5, FALSE);
  screen[1] = NewBitmap(WIDTH, HEIGHT, 4, FALSE);
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

void Kill() {
  DeleteCopList(cp);
  DeleteBitmap(screen[0]);
  DeleteBitmap(screen[1]);
  DeleteBitmap(buffer);
  DeleteBitmap(carry);
  DeletePalette(palette[0]);
  DeletePalette(palette[1]);
}

static volatile LONG swapScreen = -1;
static ULONG frameCount = 0;
static ULONG iterCount = 0;

__interrupt_handler void IntLevel3Handler() {
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

    frameCount++;
  }

  custom->intreq = INTF_LEVEL3;
  custom->intreq = INTF_LEVEL3;
}

void RotatingTriangle(WORD t, WORD phi, WORD size) {
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
  for (i = 0, j = 1; i < 3; i++, j = (i == 2 ? 0 : i + 1)) {
    BlitterLine(p[i].x, p[i].y, p[j].x, p[j].y);
    WaitBlitter();
  }
}

static void DrawShape() {
  BlitterClear(carry, 0);
  WaitBlitter();

  BlitterLineSetup(carry, 0, LINE_EOR, LINE_ONEDOT);

  RotatingTriangle(iterCount * 16, 0, SIZE);
  RotatingTriangle(iterCount * 16, SIN_PI * 2 / 3, SIZE);
  RotatingTriangle(-iterCount * 16, SIN_PI * 2 / 3, SIZE / 2);

  BlitterFill(carry, 0);
  WaitBlitter();
}

#define HALF_SUB ((SRCA | SRCB | DEST) | A_XOR_B)
#define HALF_SUB_BORROW ((SRCA | SRCB | DEST) | (NABC | NABNC))

#define A_AND_NOT_B (ANBC | ANBNC)

static void DecrementAndSaturate() {
  BitmapT *borrow = carry;
  WORD i, k;
  
  custom->bltcon1 = 0;
  custom->bltamod = 0;
  custom->bltbdat = -1;
  custom->bltbmod = 0;
  custom->bltdmod = 0;
  custom->bltafwm = -1;
  custom->bltalwm = -1;

  custom->bltapt = buffer->planes[0];
  custom->bltdpt = borrow->planes[0];
  custom->bltcon0 = HALF_SUB_BORROW & ~SRCB;
  custom->bltsize = (SIZE << 6) + (SIZE >> 4);
  WaitBlitter();

  custom->bltapt = buffer->planes[0];
  custom->bltdpt = buffer->planes[0];
  custom->bltcon0 = HALF_SUB & ~SRCB;
  custom->bltsize = (SIZE << 6) + (SIZE >> 4);
  WaitBlitter();

  for (i = 1, k = 0; i < 4; i++, k ^= 1) {
    custom->bltapt = buffer->planes[i];
    custom->bltbpt = borrow->planes[k];
    custom->bltdpt = borrow->planes[k ^ 1];
    custom->bltcon0 = HALF_SUB_BORROW;
    custom->bltsize = (SIZE << 6) + (SIZE >> 4);
    WaitBlitter();

    custom->bltapt = buffer->planes[i];
    custom->bltbpt = borrow->planes[k];
    custom->bltdpt = buffer->planes[i];
    custom->bltcon0 = HALF_SUB;
    custom->bltsize = (SIZE << 6) + (SIZE >> 4);
    WaitBlitter();
  }

  {
    WORD n = 4;

    custom->bltcon0 = (SRCA | SRCB | DEST) | A_AND_NOT_B;

    while (--n >= 0) {
      custom->bltapt = buffer->planes[n];
      custom->bltbpt = carry->planes[k];
      custom->bltdpt = buffer->planes[n];
      custom->bltsize = (SIZE << 6) + (SIZE >> 4);
      WaitBlitter();
    }
  }
}

#define HALF_ADDER ((SRCA | SRCB | DEST) | A_XOR_B)
#define HALF_ADDER_CARRY ((SRCA | SRCB | DEST) | A_AND_B)

static void IncrementAndSaturate() {
  WORD i, k;
  
  custom->bltcon1 = 0;
  custom->bltamod = 0;
  custom->bltbmod = 0;
  custom->bltdmod = 0;
  custom->bltafwm = -1;
  custom->bltalwm = -1;

  for (i = 0, k = 0; i < 4; i++, k ^= 1) {
    custom->bltapt = buffer->planes[i];
    custom->bltbpt = carry->planes[k];
    custom->bltdpt = carry->planes[k ^ 1];
    custom->bltcon0 = HALF_ADDER_CARRY;
    custom->bltsize = (SIZE << 6) + (SIZE >> 4);
    WaitBlitter();

    custom->bltapt = buffer->planes[i];
    custom->bltbpt = carry->planes[k];
    custom->bltdpt = buffer->planes[i];
    custom->bltcon0 = HALF_ADDER;
    custom->bltsize = (SIZE << 6) + (SIZE >> 4);
    WaitBlitter();
  }

  {
    WORD n = 4;

    custom->bltcon0 = (SRCA | SRCB | DEST) | A_OR_B;

    while (--n >= 0) {
      custom->bltapt = buffer->planes[n];
      custom->bltbpt = carry->planes[k];
      custom->bltdpt = buffer->planes[n];
      custom->bltsize = (SIZE << 6) + (SIZE >> 4);
      WaitBlitter();
    }
  }
}

static void CopyToScreen() {
  WORD n = 4;

  custom->bltamod = 0;
  custom->bltdmod = (WIDTH - SIZE) / 8;
  custom->bltafwm = -1;
  custom->bltalwm = -1;
  custom->bltcon0 = A_TO_D | (SRCA | DEST);
  custom->bltcon1 = 0;

  while (--n >= 0) {
    custom->bltapt = buffer->planes[n];
    custom->bltdpt = screen[active]->planes[n] + 2;
    custom->bltsize = (SIZE << 6) + (SIZE >> 4);
    WaitBlitter();
  }
}

BOOL Loop() {
  // LONG lines = ReadLineCounter();

  if (iterCount++ & 1)
    DecrementAndSaturate();
  DrawShape();
  IncrementAndSaturate();
  CopyToScreen();

  // Log("loop: %ld\n", ReadLineCounter() - lines);

  swapScreen = active;
  active ^= 1;

  return !LeftMouseButton();
}

void Main() {
  InterruptVector->IntLevel3 = IntLevel3Handler;
  custom->intena = INTF_SETCLR | INTF_VERTB;
  
  CopListActivate(cp);
  custom->dmacon = DMAF_SETCLR | DMAF_BLITTER | DMAF_RASTER | DMAF_BLITHOG;

  CircleEdge(screen[0], 4, SIZE / 2 + 16, SIZE / 2, SIZE / 4 - 1);
  BlitterFill(screen[0], 4);
  WaitBlitter();

  {
    WORD i;

    for (i = 0; i < 2; i++) {
      BlitterLineSetup(screen[i], 3, LINE_OR, LINE_SOLID);
      BlitterLine(WIDTH - 1, 0, WIDTH - 1, SIZE - 2);
      WaitBlitter();
      BlitterLine(WIDTH / 2, 0, WIDTH / 2, SIZE - 2);
      WaitBlitter();
      Circle(screen[i], 4, WIDTH * 3 / 4, SIZE / 2, SIZE / 2 - 10);
      BlitterFill(screen[i], 3);
      WaitBlitter();
    }
  }

  while (Loop());
}
