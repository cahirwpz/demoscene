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
#define DEPTH 5
#define SIZE 128

static BitmapT *screen[2];
static UWORD active = 0;

static BitmapT *clip;
static BitmapT *carry;
static BitmapT *buffer;
static PaletteT *palette[2];
static CopInsT *bplptr[2][DEPTH];
static CopListT *cp;

static void Load() {
  clip = LoadILBM("data/blurred-clip.ilbm", FALSE);

  palette[0] = LoadPalette("data/blurred1.ilbm");
  palette[1] = LoadPalette("data/blurred2.ilbm");

  screen[0] = NewBitmap(WIDTH, HEIGHT, DEPTH, FALSE);
  screen[1] = NewBitmap(WIDTH, HEIGHT, DEPTH, FALSE);
  buffer = NewBitmap(SIZE, SIZE, 4, FALSE);
  carry = NewBitmap(SIZE, SIZE, 2, FALSE);

  cp = NewCopList(200);
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

static ULONG iterCount = 0;

static void MakeCopperList(CopListT *cp) {
  WORD i;

  CopInit(cp);
  CopLoadPal(cp, palette[0], 0);
  CopMakeDispWin(cp, X(0), Y(0), WIDTH, HEIGHT);
  CopMakePlayfield(cp, bplptr[0], screen[active], DEPTH);
  CopWait(cp, Y(127), 0);
  CopLoadPal(cp, palette[1], 0);
  CopWait(cp, Y(128), 0);
  for (i = 0; i < screen[active]->depth; i++)
    bplptr[1][i] = CopMove32(cp, bplpt[i], screen[active]->planes[i] - WIDTH / 16);
  CopEnd(cp);
}

static void Init() {
  MakeCopperList(cp);
  CopListActivate(cp);
  custom->dmacon = DMAF_SETCLR | DMAF_BLITTER | DMAF_RASTER | DMAF_BLITHOG;

  /* Make the center of blurred shape use colors from range 16-31. */
  CircleEdge(screen[0], 4, SIZE / 2 + 16, SIZE / 2, SIZE / 4 - 1);
  BlitterFill(screen[0], 4);
  WaitBlitter();

  ITER(i, 0, 4, BlitterCopySync(screen[0], i, WIDTH / 2, 0, clip, i));
  ITER(i, 0, 4, BlitterCopySync(screen[1], i, WIDTH / 2, 0, clip, i));
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

static void Render() {
  //LONG lines = ReadLineCounter();

  if (iterCount++ & 1)
    DecrementAndSaturate();
  DrawShape();
  IncrementAndSaturate();

  ITER(i, 0, 3, BlitterCopySync(screen[active], i, 16, 0, buffer, i));

  //Log("loop: %ld\n", ReadLineCounter() - lines);

  WaitVBlank();
  ITER(i, 0, 3, {
    CopInsSet32(bplptr[0][i], screen[active]->planes[i]);
    CopInsSet32(bplptr[1][i], screen[active]->planes[i] - WIDTH / 16);
    });
  active ^= 1;
}

EffectT Effect = { Load, UnLoad, Init, NULL, Render };
