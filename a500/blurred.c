#include "blitter.h"
#include "coplist.h"
#include "interrupts.h"
#include "memory.h"
#include "ilbm.h"
#include "2d.h"
#include "fx.h"

#define WIDTH 320
#define HEIGHT 256
#define SIZE 128

static BitmapT *screen[2];
static UWORD active = 0;

static BitmapT *carry;
static BitmapT *buffer;
static PaletteT *palette;
static CopInsT *bplptr[5];
static CopListT *cp;

void Load() {
  palette = LoadPalette("data/blurred.ilbm");

  screen[0] = NewBitmap(WIDTH, HEIGHT, 5, FALSE);
  screen[1] = NewBitmap(WIDTH, HEIGHT, 4, FALSE);
  buffer = NewBitmap(SIZE, SIZE, 4, FALSE);
  carry = NewBitmap(SIZE, SIZE, 2, FALSE);

  cp = NewCopList(100);
  CopInit(cp);
  CopMakePlayfield(cp, bplptr, screen[active]);
  CopMakeDispWin(cp, 0x81, 0x2c, WIDTH, HEIGHT);
  CopLoadPal(cp, palette, 0);
  CopEnd(cp);
}

void Kill() {
  DeleteCopList(cp);
  DeleteBitmap(screen[0]);
  DeleteBitmap(screen[1]);
  DeleteBitmap(buffer);
  DeleteBitmap(carry);
  DeletePalette(palette);
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
    custom->bltdpt = screen[active]->planes[n];
    custom->bltsize = (SIZE << 6) + (SIZE >> 4);
    WaitBlitter();
  }
}

BOOL Loop() {
  LONG lines = ReadLineCounter();

  if (iterCount++ & 1)
    DecrementAndSaturate();
  DrawShape();
  IncrementAndSaturate();
  CopyToScreen();

  Log("loop: %ld\n", ReadLineCounter() - lines);

  swapScreen = active;
  active ^= 1;

  return !LeftMouseButton();
}

void Main() {
  InterruptVector->IntLevel3 = IntLevel3Handler;
  custom->intena = INTF_SETCLR | INTF_VERTB;
  
  CopListActivate(cp);
  custom->dmacon = DMAF_SETCLR | DMAF_BLITTER | DMAF_RASTER | DMAF_BLITHOG;

  while (Loop());
}
