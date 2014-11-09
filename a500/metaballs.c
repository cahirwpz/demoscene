#include "startup.h"
#include "blitter.h"
#include "coplist.h"
#include "memory.h"
#include "ilbm.h"
#include "2d.h"
#include "fx.h"

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
static CopInsT *bplptr[DEPTH];
static CopListT *cp;

static void Load() {
  screen[0] = NewBitmap(WIDTH, HEIGHT, DEPTH);
  screen[1] = NewBitmap(WIDTH, HEIGHT, DEPTH);

  bgLeft = LoadILBM("data/metaball-bg-left-1.ilbm");
  DeletePalette(bgLeft->palette);
  bgRight = LoadILBM("data/metaball-bg-right-1.ilbm");
  DeletePalette(bgRight->palette);
  metaball = LoadILBM("data/metaball-1.ilbm");
}

static void UnLoad() {
  DeleteBitmap(screen[0]);
  DeleteBitmap(screen[1]);
  DeleteBitmap(bgLeft);
  DeleteBitmap(bgRight);
  DeletePalette(metaball->palette);
  DeleteBitmap(metaball);
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
  CopMakePlayfield(cp, bplptr, screen[active], DEPTH);
  CopMakeDispWin(cp, X(0), Y(0), WIDTH, HEIGHT);
  CopLoadPal(cp, metaball->palette, 0);
  CopEnd(cp);
}

static void Init() {
  WORD i, j;

  custom->dmacon = DMAF_SETCLR | DMAF_BLITTER;

  for (j = 0; j < 2; j++) {
    for (i = 0; i < DEPTH; i++) {
      BlitterSetSync(screen[j], i, 32, 0, WIDTH - 64, HEIGHT, 0);
      BlitterCopySync(screen[j], i, 0, 0, bgLeft, i);
      BlitterCopySync(screen[j], i, WIDTH - 32, 0, bgRight, i);
    }
  }

  cp = NewCopList(100);
  carry = NewBitmap(SIZE + 16, SIZE, 2);

  SetInitialPositions();

  MakeCopperList(cp);
  CopListActivate(cp);
  custom->dmacon = DMAF_SETCLR | DMAF_RASTER;
}

static void Kill() {
  custom->dmacon = DMAF_COPPER | DMAF_BLITTER | DMAF_RASTER;

  DeleteBitmap(carry);
  DeleteCopList(cp);
}

#define BLTOP_NAME ClearMetaball
#define BLTOP_DST_BM screen[active]
#define BLTOP_DST_WIDTH WIDTH
#define BLTOP_HSIZE SIZE
#define BLTOP_VSIZE SIZE
#define BLTOP_BPLS 5
#include "bltop_clear_simple.h"

#define BLTOP_NAME CopyMetaball
#define BLTOP_SRC_BM metaball
#define BLTOP_DST_BM screen[active]
#define BLTOP_DST_WIDTH WIDTH
#define BLTOP_HSIZE SIZE
#define BLTOP_VSIZE SIZE
#define BLTOP_BPLS 5
#include "bltop_copy_simple.h"

#define BLTOP_NAME AddMetaball
#define BLTOP_SRC_BM metaball
#define BLTOP_SRC_WIDTH SIZE
#define BLTOP_CARRY_BM carry
#define BLTOP_DST_BM screen[active]
#define BLTOP_DST_WIDTH WIDTH
#define BLTOP_HSIZE SIZE
#define BLTOP_VSIZE SIZE
#define BLTOP_BPLS 5
#include "bltop_add_sat.h"

static void ClearMetaballs() {
  Point2D *p = pos[active];
  WORD j;

  for (j = 0; j < 3; j++, p++)
    ClearMetaball(p->x, p->y);
}

static void PositionMetaballs() {
  LONG t = frameCount * 24;

  pos[active][0].x = (WIDTH - SIZE) / 2 + normfx(SIN(t) * SIZE * 3 / 4);
  pos[active][0].y = (HEIGHT - SIZE) / 2;

  pos[active][1].x = (WIDTH - SIZE) / 2 - normfx(SIN(t) * SIZE * 3 / 4);
  pos[active][1].y = (HEIGHT - SIZE) / 2;

  pos[active][2].x = (WIDTH - SIZE) / 2;
  pos[active][2].y = (HEIGHT - SIZE) / 2 + normfx(COS(t) * SIZE * 3 / 4);
}

static void Render() {
  // LONG lines = ReadLineCounter();

  // This takes about 100 lines. Could we do better?
  ClearMetaballs();
  PositionMetaballs();

  CopyMetaball(pos[active][0].x, pos[active][0].y);
  AddMetaball(pos[active][1].x, pos[active][1].y, 0, 0);
  AddMetaball(pos[active][2].x, pos[active][2].y, 0, 0);

  // Log("loop: %ld\n", ReadLineCounter() - lines);

  WaitVBlank();
  ITER(i, 0, DEPTH - 1, CopInsSet32(bplptr[i], screen[active]->planes[i]));
  active ^= 1;
}

EffectT Effect = { Load, UnLoad, Init, Kill, Render };
