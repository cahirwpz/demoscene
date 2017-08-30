#include "startup.h"
#include "blitter.h"
#include "coplist.h"
#include "memory.h"
#include "ilbm.h"
#include "2d.h"
#include "fx.h"

STRPTR __cwdpath = "data";

#define WIDTH 320
#define HEIGHT 256
#define DEPTH 5
#define SIZE 80

static BitmapT *screen[2];
static WORD active = 0;

static Point2D pos[2][3];
static BitmapT *bgLeft, *bgRight;
static BitmapT *metaball;
static BitmapT *carry;
static PaletteT *palette;
static CopInsT *bplptr[DEPTH];
static CopListT *cp;

static void Load() {
  screen[0] = NewBitmap(WIDTH, HEIGHT, DEPTH);
  screen[1] = NewBitmap(WIDTH, HEIGHT, DEPTH);

  bgLeft = LoadILBMCustom("metaball-bg-left-1.ilbm",
                          BM_DISPLAYABLE | BM_LOAD_PALETTE);
  bgRight = LoadILBMCustom("metaball-bg-right-1.ilbm", BM_DISPLAYABLE);
  metaball = LoadILBMCustom("metaball.ilbm", BM_DISPLAYABLE);
  palette = bgLeft->palette;
}

static void UnLoad() {
  DeleteBitmap(screen[0]);
  DeleteBitmap(screen[1]);
  DeleteBitmap(bgLeft);
  DeleteBitmap(bgRight);
  DeleteBitmap(metaball);
  DeletePalette(palette);
}

static void SetInitialPositions() {
  WORD i, j;

  for (i = 0; i < 2; i++) {
    for (j = 0; j < 3; j++) {
      pos[i][j].x = 160;
      pos[i][j].x = 128;
    }
  }
}

static void MakeCopperList(CopListT *cp) {
  CopInit(cp);
  CopSetupGfxSimple(cp, MODE_LORES, DEPTH, X(0), Y(0), WIDTH, HEIGHT);
  CopSetupBitplanes(cp, bplptr, screen[active], DEPTH);
  CopLoadPal(cp, palette, 0);
  CopEnd(cp);
}

static void Init() {
  WORD j;

  EnableDMA(DMAF_BLITTER | DMAF_BLITHOG);

  for (j = 0; j < 2; j++) {
    BitmapClearArea(screen[j], STRUCT(Area2D, 32, 0, WIDTH - 64, HEIGHT));
    BitmapCopy(screen[j], 0, 0, bgLeft);
    BitmapCopy(screen[j], WIDTH - 32, 0, bgRight);
  }

  cp = NewCopList(100);
  carry = NewBitmap(SIZE + 16, SIZE, 2);

  SetInitialPositions();

  MakeCopperList(cp);
  CopListActivate(cp);
  EnableDMA(DMAF_RASTER);
}

static void Kill() {
  DisableDMA(DMAF_COPPER | DMAF_RASTER | DMAF_BLITTER | DMAF_BLITHOG);

  DeleteBitmap(carry);
  DeleteCopList(cp);
}

static void ClearMetaballs() {
  Area2D mball = {0, 0, SIZE + 16, SIZE};
  WORD *val = (WORD *)pos[active];
  WORD n = 3;

  while (--n >= 0) {
    mball.x = *val++ & ~15;
    mball.y = *val++;
    BitmapClearArea(screen[active], &mball);
  }
}

static void PositionMetaballs() {
  LONG t = frameCount * 24;
  WORD *val = (WORD *)pos[active];


  *val++ = (WIDTH - SIZE) / 2 + normfx(SIN(t) * SIZE * 3 / 4);
  *val++ = (HEIGHT - SIZE) / 2;
  *val++ = (WIDTH - SIZE) / 2 - normfx(SIN(t) * SIZE * 3 / 4);
  *val++ = (HEIGHT - SIZE) / 2;
  *val++ = (WIDTH - SIZE) / 2;
  *val++ = (HEIGHT - SIZE) / 2 + normfx(COS(t) * SIZE * 3 / 4);
}

static void DrawMetaballs() {
  WORD *val = (WORD *)pos[active];
  LONG x, y;

  x = *val++; y = *val++; BitmapCopyFast(screen[active], x, y, metaball);
  x = *val++; y = *val++; BitmapAddSaturated(screen[active], x, y, metaball, carry);
  x = *val++; y = *val++; BitmapAddSaturated(screen[active], x, y, metaball, carry);
}

static void Render() {
  // LONG lines = ReadLineCounter();

  // This takes about 100 lines. Could we do better?
  ClearMetaballs();
  PositionMetaballs();
  DrawMetaballs();

  // Log("metaballs : %ld\n", ReadLineCounter() - lines);

  WaitVBlank();
  ITER(i, 0, DEPTH - 1, CopInsSet32(bplptr[i], screen[active]->planes[i]));
  active ^= 1;
}

EffectT Effect = { Load, UnLoad, Init, Kill, Render };
