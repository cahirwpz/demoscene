#include <effect.h>
#include <blitter.h>
#include <circle.h>
#include <copper.h>
#include <fx.h>
#include <gfx.h>
#include <sprite.h>
#include <system/memory.h>

#define WIDTH 256
#define HEIGHT 256
#define DEPTH 3
#define SIZE 16
#define NUM 37
#define ARMS 3

#define MAX (96+16)

static __code CopListT *cp;
static __code CopInsPairT *bplptr;
static __code CopInsPairT *sprptr;
static __code BitmapT *screen[2];
static __code short active = 0;
static __code BitmapT *carry;
static __code BitmapT *flare[8];

#include "data/plotter-flares.c"
#include "data/background-1.c"
#include "data/background-2.c"
#include "data/background-3.c"
#include "data/gradient.c"

static CopListT *MakeCopperList(void) {
  CopListT *cp = NewCopList(50 + HEIGHT * (9 + 9 + 13));
  u_short *cols = background_cols_pixels;
  short i, j;

  sprptr = CopSetupSprites(cp);
  for (i = 0; i < 8; i++) {
    CopInsSetSprite(&sprptr[i], background_1[i]);
    SpriteUpdatePos(background_1[i], X(16 * i), Y(0), background_1_info);
  }

  CopWait(cp, Y(-1), HP(0));
  bplptr = CopSetupBitplanes(cp, screen[active], DEPTH);

  for (i = 0; i < HEIGHT; i++) {
    u_short c0, c1, c2;

    CopWaitSafe(cp, Y(i), HP(0));

    for (j = 0; j < 8; j++)
      CopMove16(cp, spr[j].pos, SPRPOS(DIWHP + 16*j + 32, DIWVP + i));

    cols++;
    c0 = *cols++;
    c1 = *cols++;
    c2 = *cols++;

    CopMove16(cp, color[17], c0);
    CopMove16(cp, color[18], c1);
    CopMove16(cp, color[19], c2);
    CopMove16(cp, color[21], c0);
    CopMove16(cp, color[22], c1);
    CopMove16(cp, color[23], c2);
    CopMove16(cp, color[25], c0);
    CopMove16(cp, color[26], c1);
    CopMove16(cp, color[27], c2);
    CopMove16(cp, color[29], c0);
    CopMove16(cp, color[30], c1);
    CopMove16(cp, color[31], c2);

    CopWaitSafe(cp, Y(i), X(128));
    for (j = 0; j < 8; j++)
      CopMove16(cp, spr[j].pos, SPRPOS(DIWHP + 16*j + 128 + 32, DIWVP + i));
  }

  CopListFinish(cp);
  return cp;
}

static void Init(void) {
  short i;

  screen[0] = NewBitmap(WIDTH, HEIGHT, DEPTH, BM_CLEAR);
  screen[1] = NewBitmap(WIDTH, HEIGHT, DEPTH, BM_CLEAR);

  EnableDMA(DMAF_BLITTER);

  for (i = 0; i < 8; i++) {
    Area2D flare_area = { 0, i * SIZE, SIZE, SIZE };
    flare[i] = NewBitmap(SIZE, SIZE, DEPTH, 0);
    BitmapClear(flare[i]);
    BitmapCopyArea(flare[i], 0, 0, &flares, &flare_area);
  }

  carry = NewBitmap(SIZE + 16, SIZE, 2, 0);

  for (i = 0; i < 2; i++)
    BitmapClear(screen[i]);

  SetupPlayfield(MODE_LORES, DEPTH, X((320 - WIDTH) / 2), Y(0), WIDTH, HEIGHT);
  LoadColors(flares_colors, 0);
  LoadColors(background_colors, 16);
  LoadColors(background_colors, 20);
  LoadColors(background_colors, 24);
  LoadColors(background_colors, 28);

  custom->bplcon2 = 0;

  cp = MakeCopperList();
  CopListActivate(cp);
  EnableDMA(DMAF_RASTER | DMAF_SPRITE | DMAF_BLITHOG);
}

static void Kill(void) {
  DisableDMA(DMAF_COPPER | DMAF_BLITTER | DMAF_RASTER | DMAF_SPRITE | DMAF_BLITHOG);

  ITER(i, 0, 7, DeleteBitmap(flare[i]));
  DeleteBitmap(carry);
  DeleteBitmap(screen[0]);
  DeleteBitmap(screen[1]);
  DeleteCopList(cp);
}

static void DrawPlotter(BitmapT *screen, short frameCount) {
  short i, a;
  short t = frameCount * 5;
  short da = 2 * SIN_PI / NUM;

  for (i = 0, a = t; i < NUM; i++, a += da) {
    short g = SIN(ARMS * a);
    short x = normfx(normfx(SIN(t + a) * g) * MAX) + 128 - SIZE / 2;
    short y = normfx(normfx(COS(t + a) * g) * MAX) + 128 - SIZE / 2;
    short f = normfx(g * 3);

    if (f < 0)
      f = -f;

    if ((i & 1) && (frameCount & 15) < 3)
      f = 7;

    BitmapAddSaturated(screen, x, y, flare[f], carry);
  }
}

static __code SprDataT **background[3] = {
  background_1,
  background_2,
  background_3,
};

PROFILE(Draw);

static void Render(void) {
  ProfilerStart(Draw);
  {
    BitmapClear(screen[active]);
    DrawPlotter(screen[active], frameCount);
  }
  ProfilerStop(Draw);

  ITER(i, 0, DEPTH - 1, CopInsSet32(&bplptr[i], screen[active]->planes[i]));
  TaskWaitVBlank();
  active ^= 1;

  {
    short i;
    short num = mod16(div16(frameCount, 5), 3);

    for (i = 0; i < 8; i++) {
      CopInsSetSprite(&sprptr[i], background[num][i]);
      SpriteUpdatePos(background[num][i], X(16 * i), Y(0), background_1_info);
    }
  }
}

EFFECT(Plotter, NULL, NULL, Init, Kill, Render, NULL);
