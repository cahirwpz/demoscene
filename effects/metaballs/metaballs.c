#include "startup.h"
#include "blitter.h"
#include "coplist.h"
#include "memory.h"
#include "2d.h"
#include "fx.h"
#include "tasks.h"

#define WIDTH 320
#define HEIGHT 256
#define DEPTH 5
#define SIZE 80

static BitmapT *screen[2];
static short active = 0;

static Point2D pos[2][3];
static BitmapT *carry;
static CopInsT *bplptr[DEPTH];
static CopListT *cp;

#include "data/metaball.c"
#include "data/metaball-bg-left-1.c"
#include "data/metaball-bg-right-1.c"

static void Load(void) {
  screen[0] = NewBitmap(WIDTH, HEIGHT, DEPTH);
  screen[1] = NewBitmap(WIDTH, HEIGHT, DEPTH);
}

static void UnLoad(void) {
  DeleteBitmap(screen[0]);
  DeleteBitmap(screen[1]);
}

static void SetInitialPositions(void) {
  short i, j;

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
  CopLoadPal(cp, &metaball_pal, 0);
  CopEnd(cp);
}

static void Init(void) {
  short j;

  EnableDMA(DMAF_BLITTER | DMAF_BLITHOG);

  for (j = 0; j < 2; j++) {
    BitmapClearArea(screen[j], STRUCT(Area2D, 32, 0, WIDTH - 64, HEIGHT));
    BitmapCopy(screen[j], 0, 0, &bgLeft);
    BitmapCopy(screen[j], WIDTH - 32, 0, &bgRight);
  }

  cp = NewCopList(100);
  carry = NewBitmap(SIZE + 16, SIZE, 2);

  SetInitialPositions();

  MakeCopperList(cp);
  CopListActivate(cp);
  EnableDMA(DMAF_RASTER);
}

static void Kill(void) {
  DisableDMA(DMAF_COPPER | DMAF_RASTER | DMAF_BLITTER | DMAF_BLITHOG);

  DeleteBitmap(carry);
  DeleteCopList(cp);
}

static void ClearMetaballs(void) {
  Area2D mball = {0, 0, SIZE + 16, SIZE};
  short *val = (short *)pos[active];
  short n = 3;

  while (--n >= 0) {
    mball.x = *val++ & ~15;
    mball.y = *val++;
    BitmapClearArea(screen[active], &mball);
  }
}

static void PositionMetaballs(void) {
  int t = frameCount * 24;
  short *val = (short *)pos[active];


  *val++ = (WIDTH - SIZE) / 2 + normfx(SIN(t) * SIZE * 3 / 4);
  *val++ = (HEIGHT - SIZE) / 2;
  *val++ = (WIDTH - SIZE) / 2 - normfx(SIN(t) * SIZE * 3 / 4);
  *val++ = (HEIGHT - SIZE) / 2;
  *val++ = (WIDTH - SIZE) / 2;
  *val++ = (HEIGHT - SIZE) / 2 + normfx(COS(t) * SIZE * 3 / 4);
}

static void DrawMetaballs(void) {
  short *val = (short *)pos[active];
  int x, y;

  x = *val++; y = *val++; BitmapCopyFast(screen[active], x, y, &metaball);
  x = *val++; y = *val++; BitmapAddSaturated(screen[active], x, y, &metaball, carry);
  x = *val++; y = *val++; BitmapAddSaturated(screen[active], x, y, &metaball, carry);
}

static void Render(void) {
  // int lines = ReadLineCounter();

  // This takes about 100 lines. Could we do better?
  ClearMetaballs();
  PositionMetaballs();
  DrawMetaballs();

  // Log("metaballs : %d\n", ReadLineCounter() - lines);

  ITER(i, 0, DEPTH - 1, CopInsSet32(bplptr[i], screen[active]->planes[i]));
  TaskWait(VBlankEvent);
  active ^= 1;
}

EffectT Effect = { Load, UnLoad, Init, Kill, Render, NULL };
