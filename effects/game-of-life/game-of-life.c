#include "effect.h"
#include "bitmap.h"
#include "copper.h"
#include "blitter.h"

#define DISP_WIDTH 320
#define DISP_HEIGHT 256
#define BOARD_WIDTH (DISP_WIDTH+32)
#define BOARD_HEIGHT (DISP_HEIGHT+2)
#define BOARD_DEPTH 1

#include "data/initial-board.c"

static BitmapT* lo;
static BitmapT* hi;
static BitmapT* x0;
static BitmapT* x1;
static BitmapT* x2;
static BitmapT* x3;
static BitmapT* x5;
static BitmapT* x6;
static BitmapT* x7;
static CopListT* cp;

static u_short minterms_table[9] = {0x96, 0xE8, 0x7E, 0x69, 0x69, 0x81, 0xB4, 0x3A, 0x24};

static PaletteT palette = {
  .count = 2,
  .colors = {
    0x000,
    0xfff
  }
};

// setup blitter to calculate a function of three horizontally adjacent lit pixels
static void BlitAdjacentHorizontal(const BitmapT* source, BitmapT* target, u_short minterms)
{
  void* srcCenter = source->planes[0] + source->bytesPerRow; // C channel
  void* srcRight = source->planes[0] + source->bytesPerRow + 2; // B channel
  void* srcLeft = source->planes[0] + source->bytesPerRow; // A channel
  u_short bltsize = (DISP_HEIGHT << 6) | ((BOARD_WIDTH / 16));

  WaitBlitter();

  custom->bltcon0 = ASHIFT(1) | minterms | (SRCA | SRCB | SRCC | DEST);
  custom->bltcon1 = BSHIFT(15);
  
  custom->bltafwm = 0x0000;
  custom->bltalwm = 0x0000;
  custom->bltamod = 0;
  custom->bltbmod = 0;
  custom->bltcmod = 0;
  custom->bltdmod = 0;
  custom->bltapt = srcLeft;
  custom->bltbpt = srcRight;
  custom->bltcpt = srcCenter;
  custom->bltdpt = target->planes[0] + target->bytesPerRow;
  custom->bltsize = bltsize;
}

// setup blitter to calculate a function of three vertically adjacent lit pixels
static void BlitAdjacentVertical(const BitmapT* source, BitmapT* target, u_short minterms)
{
  void* srcCenter = source->planes[0] + source->bytesPerRow; // C channel
  void* srcUp = source->planes[0]; // A channel
  void* srcDown = source->planes[0] + 2*source->bytesPerRow; // B channel
  u_short bltsize = (DISP_HEIGHT << 6) | ((BOARD_WIDTH / 16));

  WaitBlitter();

  custom->bltcon0 = minterms | (SRCA | SRCB | SRCC | DEST);
  custom->bltcon1 = 0;
  
  custom->bltafwm = 0x0000;
  custom->bltalwm = 0x0000;
  custom->bltamod = 0;
  custom->bltbmod = 0;
  custom->bltcmod = 0;
  custom->bltdmod = 0;
  custom->bltapt = srcUp;
  custom->bltbpt = srcDown;
  custom->bltcpt = srcCenter;
  custom->bltdpt = target->planes[0] + target->bytesPerRow;
  custom->bltsize = bltsize;
}

static void BlitFunc(const BitmapT* sourceA, const BitmapT* sourceB, const BitmapT* sourceC, const BitmapT* target, u_short minterms)
{
  u_short bltsize = (DISP_HEIGHT << 6) | ((BOARD_WIDTH / 16));

  WaitBlitter();

  custom->bltcon0 = minterms | (SRCA | SRCB | SRCC | DEST);
  custom->bltcon1 = 0;
  
  custom->bltafwm = 0x0000;
  custom->bltalwm = 0x0000;
  custom->bltamod = 0;
  custom->bltbmod = 0;
  custom->bltcmod = 0;
  custom->bltdmod = 0;
  custom->bltapt = sourceA->planes[0] + sourceA->bytesPerRow;
  custom->bltbpt = sourceB->planes[0] + sourceB->bytesPerRow;
  custom->bltcpt = sourceC->planes[0] + sourceC->bytesPerRow;
  custom->bltdpt = target->planes[0] + target->bytesPerRow;
  custom->bltsize = bltsize;
}

static void Init(void) {
  lo = NewBitmap(BOARD_WIDTH, BOARD_HEIGHT, BOARD_DEPTH);
  hi = NewBitmap(BOARD_WIDTH, BOARD_HEIGHT, BOARD_DEPTH);
  x0 = NewBitmap(BOARD_WIDTH, BOARD_HEIGHT, BOARD_DEPTH);
  x1 = NewBitmap(BOARD_WIDTH, BOARD_HEIGHT, BOARD_DEPTH);
  x2 = NewBitmap(BOARD_WIDTH, BOARD_HEIGHT, BOARD_DEPTH);
  x3 = NewBitmap(BOARD_WIDTH, BOARD_HEIGHT, BOARD_DEPTH);
  x5 = NewBitmap(BOARD_WIDTH, BOARD_HEIGHT, BOARD_DEPTH);
  x6 = NewBitmap(BOARD_WIDTH, BOARD_HEIGHT, BOARD_DEPTH);
  x7 = NewBitmap(BOARD_WIDTH, BOARD_HEIGHT, BOARD_DEPTH);

  SetupPlayfield(MODE_LORES, BOARD_DEPTH, X(0), Y(0), DISP_WIDTH, DISP_HEIGHT);
  LoadPalette(&palette, 0);

  EnableDMA(DMAF_BLITTER | DMAF_BLITHOG);

  cp = NewCopList(80);
  CopInit(cp);
  CopMove32(cp, bplpt[0], initial_board.planes[0] + initial_board.bytesPerRow + 2);
  CopMove16(cp, bpl1mod, 4);
  CopMove16(cp, bpl2mod, 4);
  //CopSetupBitplanes(cp, NULL, &initial_board, BOARD_DEPTH);
  CopEnd(cp);
  CopListActivate(cp);

  EnableDMA(DMAF_RASTER);
}

static void Kill(void) {
  DisableDMA(DMAF_COPPER | DMAF_RASTER | DMAF_BLITTER | DMAF_SPRITE);
  DeleteCopList(cp);
  DeleteBitmap(lo);
  DeleteBitmap(hi);
  DeleteBitmap(x0);
  DeleteBitmap(x1);
  DeleteBitmap(x2);
  DeleteBitmap(x3);
  DeleteBitmap(x5);
  DeleteBitmap(x6);
  DeleteBitmap(x7);
}

PROFILE(GOLStep)

static void Render(void) {
  ProfilerStart(GOLStep);

  BlitAdjacentHorizontal(&initial_board, lo, minterms_table[0]);
  BlitAdjacentHorizontal(&initial_board, hi, minterms_table[1]);

  BlitAdjacentVertical(lo, x0, minterms_table[2]);
  BlitAdjacentVertical(lo, x1, minterms_table[3]);

  BlitAdjacentVertical(hi, x2, minterms_table[4]);
  BlitAdjacentVertical(hi, x3, minterms_table[5]);

  BlitFunc(x2, &initial_board, x3, x5, minterms_table[6]);
  BlitFunc(x1, x5, x3, x6, minterms_table[7]);
  BlitFunc(x2, x0, x6, &initial_board, minterms_table[8]);

  ProfilerStop(GOLStep);

  TaskWaitVBlank();
}

EFFECT(game_of_life, NULL, NULL, Init, Kill, Render);
