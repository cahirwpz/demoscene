#include "startup.h"
#include "blitter.h"
#include "coplist.h"
#include "memory.h"
#include "ilbm.h"
#include "2d.h"
#include "fx.h"
#include "circle.h"

STRPTR __cwdpath = "data";

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
  clip = LoadILBM("blurred-clip.ilbm");

  palette[0] = LoadPalette("blurred-pal-1.ilbm");
  palette[1] = LoadPalette("blurred-pal-2.ilbm");

  screen[0] = NewBitmap(WIDTH, HEIGHT, DEPTH);
  screen[1] = NewBitmap(WIDTH, HEIGHT, DEPTH);
}

static void UnLoad() {
  DeletePalette(clip->palette);
  DeleteBitmap(clip);
  DeleteBitmap(screen[0]);
  DeleteBitmap(screen[1]);
  DeletePalette(palette[0]);
  DeletePalette(palette[1]);
}

static WORD iterCount = 0;

static void MakeCopperList(CopListT *cp) {
  WORD i;

  CopInit(cp);
  CopSetupGfxSimple(cp, MODE_LORES, DEPTH, X(0), Y(0), WIDTH, HEIGHT);
  CopSetupBitplanes(cp, bplptr[active], screen[active], DEPTH);
  CopWait(cp, Y(-18), 0);
  CopLoadPal(cp, palette[0], 0);
  CopWait(cp, Y(127), 0);
  CopMove16(cp, dmacon, DMAF_RASTER);
  CopLoadPal(cp, palette[1], 0);
  CopWait(cp, Y(128), 0);
  CopMove16(cp, dmacon, DMAF_SETCLR | DMAF_RASTER);
  for (i = 0; i < DEPTH; i++)
    bplptr[1][i] = CopMove32(cp, bplpt[i], screen[active]->planes[i] - WIDTH / 16);
  CopEnd(cp);
}

static void Init() {
  WORD i;

  custom->dmacon = DMAF_SETCLR | DMAF_BLITTER | DMAF_BLITHOG;

  for (i = 0; i < 2; i++) {
    BitmapClear(screen[i]);
    WaitBlitter();

    /* Make the center of blurred shape use colors from range 16-31. */
    CircleEdge(screen[i], 4, SIZE / 2 + 16, SIZE / 2, SIZE / 4 - 1);
    BlitterFill(screen[i], 4);

    BitmapCopy(screen[i], WIDTH / 2, 0, clip);
  }

  buffer = NewBitmap(SIZE, SIZE, 4);
  carry = NewBitmap(SIZE, SIZE, 2);

  cp = NewCopList(200);
  MakeCopperList(cp);
  CopListActivate(cp);
  custom->dmacon = DMAF_SETCLR | DMAF_RASTER;
}

static void Kill() {
  custom->dmacon = DMAF_COPPER | DMAF_RASTER | DMAF_BLITTER | DMAF_BLITHOG;

  DeleteCopList(cp);
  DeleteBitmap(carry);
  DeleteBitmap(buffer);
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
    BlitterLine(p[i].x, p[i].y, p[j].x, p[j].y);
}

static void DrawShape() {
  BlitterClear(carry, 0);
  BlitterLineSetup(carry, 0, LINE_EOR|LINE_ONEDOT);

  RotatingTriangle(iterCount * 16, 0, SIZE - 1);
  RotatingTriangle(iterCount * 16, SIN_PI * 2 / 3, SIZE - 1);
  RotatingTriangle(-iterCount * 16, SIN_PI * 2 / 3, SIZE / 2 - 1);

  BlitterFill(carry, 0);
}

static void Render() {
  //LONG lines = ReadLineCounter();

  if (iterCount++ & 1)
    BitmapDecSaturated(buffer, carry);

  DrawShape();
  BitmapIncSaturated(buffer, carry);

  BitmapCopy(screen[active], 16, 0, buffer);

  //Log("blurred: %ld\n", ReadLineCounter() - lines);

  WaitVBlank();
  ITER(i, 0, DEPTH - 1, {
    CopInsSet32(bplptr[0][i], screen[active]->planes[i]);
    CopInsSet32(bplptr[1][i], screen[active]->planes[i] - WIDTH / 16);
    });
  active ^= 1;
}

EffectT Effect = { Load, UnLoad, Init, Kill, Render };
