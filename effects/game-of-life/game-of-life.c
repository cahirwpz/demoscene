#include "effect.h"
#include "bitmap.h"
#include "copper.h"
#include "blitter.h"
#include "interrupt.h"
#include "memory.h"

#define DISP_WIDTH 320
#define DISP_HEIGHT 256
#define DISP_DEPTH 4

#define BOARD_WIDTH (DISP_WIDTH/2)
#define BOARD_HEIGHT (DISP_HEIGHT/2)
#define EXT_BOARD_WIDTH (BOARD_WIDTH+32)
#define EXT_BOARD_HEIGHT (BOARD_HEIGHT+2)
#define BOARD_DEPTH 1

#include "data/initial-board.c"
#include "double_pixels.h"

static BitmapT* lo;
static BitmapT* hi;
static BitmapT* x0;
static BitmapT* x1;
static BitmapT* x2;
static BitmapT* x3;
static BitmapT* x5;
static BitmapT* x6;
static BitmapT* doubled;
static CopListT* cp0;
static CopInsT* bplptr[DISP_DEPTH];
static BitmapT* prev_states[DISP_DEPTH]; // circular buffer of previous states
u_short states_head = 0; // states_head & 0x3 points to the newest frame in prev_state
u_short phase = 0; // phase (0-8) of blitter calculations

// TODO: promień śmierci/promień życia
// TODO: zoomowanie się na fragmenty
// TODO: wblitowywanie ciekawych patternów

static u_short minterms_table[9] = {0x96, 0xE8, 0x7E, 0x69, 0x69, 0x81, 0xB4, 0x3A, 0x24};

static PaletteT palette = {
  .count = 16,
  .colors = {
    0x000,
    0x001+5,
    0x011+5,
    0x023+5,
    0x034+4,
    0x057+4,
    0x079+4,
    0x09C+3,
    0x0BF,
    0x09C+3,
    0x079+5,
    0x057+4,
    0x034+4,
    0x023+5,
    0x011+5,
    0x001+5,
  }
};

// setup blitter to calculate a function of three horizontally adjacent lit pixels
static void BlitAdjacentHorizontal(const BitmapT* source, BitmapT* target, u_short minterms)
{
  void* srcCenter = source->planes[0] + source->bytesPerRow; // C channel
  void* srcRight = source->planes[0] + source->bytesPerRow + 2; // B channel
  void* srcLeft = source->planes[0] + source->bytesPerRow; // A channel
  u_short bltsize = (BOARD_HEIGHT << 6) | ((EXT_BOARD_WIDTH / 16));

  //WaitBlitter();

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
  u_short bltsize = (BOARD_HEIGHT << 6) | ((EXT_BOARD_WIDTH / 16));

  //WaitBlitter();

  custom->bltcon0 = minterms | (SRCA | SRCB | SRCC | DEST);
  custom->bltcon1 = 0;
  
  custom->bltafwm = 0xFFFF;
  custom->bltalwm = 0xFFFF;
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
  u_short bltsize = (BOARD_HEIGHT << 6) | ((EXT_BOARD_WIDTH / 16));

  //WaitBlitter();

  custom->bltcon0 = minterms | (SRCA | SRCB | SRCC | DEST);
  custom->bltcon1 = 0;
  
  custom->bltafwm = 0xFFFF;
  custom->bltalwm = 0xFFFF;
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

static void DoublePixelsHorizontal(const BitmapT* source, BitmapT* target)
{
  // u_short x;
  // u_short y;
  // u_char* double_src = (u_char*)(source->planes[0] + source->bytesPerRow + 2);
  // u_short* double_target = (u_short*)(target->planes[0]);
  // for (y = 1; y < source->height - 1; y++)
  // {
  //   for (x = 2; x < source->bytesPerRow - 2; x++)
  //     *double_target++ = double_pixels[*double_src++];
  //   double_src += 4; // source modulo - skip 4 bytes (2 on the left, 2 on the right on the next row)
  // }
  u_short x;
  u_short y;
  u_char* double_src = (u_char*)(source->planes[0] + source->bytesPerRow + 2);
  u_short* double_target = (u_short*)(target->planes[0]);
  short n = (BOARD_WIDTH/8) * BOARD_HEIGHT;
  short i = 0;

  while (--n >= 0) {
    *double_target++ = double_pixels[*double_src++];
    i++;
    if (i == 20)
    {
      i = 0;
      double_src += 4;
    }
  }
}

static void (*PixelDouble)(u_char* source asm("a0"), u_short* target asm("a1"), u_short* lut asm("a2"));
#define PixelDoubleSize (BOARD_WIDTH * BOARD_HEIGHT * 16 + 2)

static void MakePixelDoublingCode(const BitmapT* source, const BitmapT* target)
{
  u_short x;
  u_short y;
  u_short *code = (void*)PixelDouble;
  u_char* double_src = (u_char*)(source->planes[0] + source->bytesPerRow + 2);
  u_short* double_target = (u_short*)(target->planes[0]);

  *code++ = 0x7200;
  // 1b6:	7200           	moveq #0,d1
  for (y = 1; y < source->height - 1; y++)
  {
    for (x = 2; x < source->bytesPerRow - 2; x++)
    {
      *code++ = 0x1218;
      *code++ = 0x2001;
      *code++ = 0xd081;
      *code++ = 0x32f2;
      *code++ = 0x0800;
      // 1be:	1218           	move.b (a0)+,d1
      // 1c0:	2001           	move.l d1,d0
      // 1c2:	d081           	add.l d1,d0
      // 1c4:	32f2 0800      	move.w (0,a2,d0.l),(a1)+
      // *double_target++ = double_pixels[*double_src++];
    }
    // 1d2:	5888           	addq.l #4,a0
    // double_src += 4; // source modulo - skip 4 bytes (2 on the left, 2 on the right on the next row)
    *code++ = 0x5888;
  }

  *code++ = 0x4e75; /* rts */
}

static void MakeCopperList(CopListT* cp) {
  // newest game state is on (DISP_DEPTH-1)th bitplane, oldest on 0th bitplane
  BitmapT* cur;
  u_short i;
  
  CopInit(cp);
  for (i = 0; i < DISP_DEPTH; i++)
  {
    cur = prev_states[(states_head + i + 1) % DISP_DEPTH];
    CopMove32(cp, bplpt[i], cur->planes[0] + cur->bytesPerRow + 2);
  }

  CopMove16(cp, bpl1mod, 4);
  CopMove16(cp, bpl2mod, 4);
  CopEnd(cp);
}

static void UpdateBitplanePointers(void)
{
  BitmapT* cur;
  u_short i;
  for (i = 0; i < DISP_DEPTH; i++)
  {
    cur = prev_states[(states_head + i + 1) % DISP_DEPTH];
    CopInsSet32(bplptr[i], cur->planes[0]);
  }
}

static void GameOfLife(void)
{
  ClearIRQ(INTF_BLIT);
  switch (phase)
  {
    case 0: BlitAdjacentHorizontal(&initial_board, lo, minterms_table[0]); break;
    case 1: BlitAdjacentHorizontal(&initial_board, hi, minterms_table[1]); break;

    case 2: BlitAdjacentVertical(lo, x0, minterms_table[2]); break;
    case 3: BlitAdjacentVertical(lo, x1, minterms_table[3]); break;

    case 4: BlitAdjacentVertical(hi, x2, minterms_table[4]); break;
    case 5: BlitAdjacentVertical(hi, x3, minterms_table[5]); break;

    case 6: BlitFunc(x2, &initial_board, x3, x5, minterms_table[6]); break;
    case 7: BlitFunc(x1, x5, x3, x6, minterms_table[7]); break;
    case 8: BlitFunc(x2, x0, x6, &initial_board, minterms_table[8]); break;
    case 9: WaitVBlank();
  }
  phase++;
}

static void Init(void) {
  u_short i;

  lo = NewBitmap(EXT_BOARD_WIDTH, EXT_BOARD_HEIGHT, BOARD_DEPTH);
  hi = NewBitmap(EXT_BOARD_WIDTH, EXT_BOARD_HEIGHT, BOARD_DEPTH);
  x0 = NewBitmap(EXT_BOARD_WIDTH, EXT_BOARD_HEIGHT, BOARD_DEPTH);
  x1 = NewBitmap(EXT_BOARD_WIDTH, EXT_BOARD_HEIGHT, BOARD_DEPTH);
  x2 = NewBitmap(EXT_BOARD_WIDTH, EXT_BOARD_HEIGHT, BOARD_DEPTH);
  x3 = NewBitmap(EXT_BOARD_WIDTH, EXT_BOARD_HEIGHT, BOARD_DEPTH);
  x5 = NewBitmap(EXT_BOARD_WIDTH, EXT_BOARD_HEIGHT, BOARD_DEPTH);
  x6 = NewBitmap(EXT_BOARD_WIDTH, EXT_BOARD_HEIGHT, BOARD_DEPTH);

  SetupPlayfield(MODE_LORES, DISP_DEPTH, X(0), Y(0), DISP_WIDTH, DISP_HEIGHT);
  LoadPalette(&palette, 0);

  EnableDMA(DMAF_BLITTER);

  for (i = 0; i < DISP_DEPTH; i++)
  {
    prev_states[i] = NewBitmap(DISP_WIDTH, DISP_HEIGHT, BOARD_DEPTH);
    BitmapClear(prev_states[i]);
  }

  PixelDouble = MemAlloc(PixelDoubleSize, MEMF_PUBLIC);
  MakePixelDoublingCode(&initial_board, prev_states[0]->planes[0]);

  cp0 = NewCopList(300);
  CopInit(cp0);
  
  bplptr[0] = CopMove32(cp0, bplpt[0], prev_states[0]->planes[0]);
  bplptr[1] = CopMove32(cp0, bplpt[1], prev_states[1]->planes[0]);
  bplptr[2] = CopMove32(cp0, bplpt[2], prev_states[2]->planes[0]);
  bplptr[3] = CopMove32(cp0, bplpt[3], prev_states[3]->planes[0]);
  for (i = 1; i <= DISP_HEIGHT; i += 2)
  {
    CopMove16(cp0, bpl1mod, -prev_states[0]->bytesPerRow);
    CopMove16(cp0, bpl2mod, -prev_states[0]->bytesPerRow);
    CopWaitSafe(cp0, Y(i), 0);
    CopMove16(cp0, bpl1mod, 0);
    CopMove16(cp0, bpl2mod, 0);
    CopWaitSafe(cp0, Y(i+1), 0);
  }
  CopEnd(cp0);
  CopListActivate(cp0);

  EnableDMA(DMAF_RASTER);

  SetIntVector(BLIT, (IntHandlerT)GameOfLife, NULL);
  EnableINT(INTF_BLIT);
}

static void Kill(void) {
  u_short i;
  DisableDMA(DMAF_COPPER | DMAF_RASTER | DMAF_BLITTER);
  DisableINT(INTF_BLIT);
  ResetIntVector(BLIT);
  DeleteCopList(cp0);
  DeleteBitmap(lo);
  DeleteBitmap(hi);
  DeleteBitmap(x0);
  DeleteBitmap(x1);
  DeleteBitmap(x2);
  DeleteBitmap(x3);
  DeleteBitmap(x5);
  DeleteBitmap(x6);
  for (i = 0; i < DISP_DEPTH; i++)
    DeleteBitmap(prev_states[i]);
}

PROFILE(GOLStep)

static void Render(void) {
  ProfilerStart(GOLStep);
    states_head++;
    PixelDouble(initial_board.planes[0] + initial_board.bytesPerRow + 2, prev_states[states_head % DISP_DEPTH]->planes[0], double_pixels);
    UpdateBitplanePointers();
    phase = 0;
    GameOfLife();
  ProfilerStop(GOLStep);
}

EFFECT(game_of_life, NULL, NULL, Init, Kill, Render);
