#include "effect.h"
#include "bitmap.h"
#include "copper.h"
#include "blitter.h"
#include "interrupt.h"
#include "memory.h"

#define DISP_WIDTH 320
#define DISP_HEIGHT 256
#define DISP_DEPTH 4

#define START_OFFSET_X 16
#define START_OFFSET_Y 1

#define BOARD_WIDTH (DISP_WIDTH/2)
#define BOARD_HEIGHT (DISP_HEIGHT/2)
#define EXT_BOARD_WIDTH (BOARD_WIDTH+2*START_OFFSET_X)
#define EXT_BOARD_HEIGHT (BOARD_HEIGHT+2*START_OFFSET_Y)
#define BOARD_DEPTH 1

#include "data/current-board.c"
#include "data/p46basedprng.c"
#include "double_pixels.h"

static CopListT* cp;

// intermediate results
static BitmapT* lo;
static BitmapT* hi;
static BitmapT* x0;
static BitmapT* x1;
static BitmapT* x2;
static BitmapT* x3;
static BitmapT* x5;
static BitmapT* x6;

// pointers to copper instructions, for rewriting bitplane pointers
static CopInsT* bplptr[DISP_DEPTH];

// circular buffer of previous game states as they would be rendered (with horizontally doubled pixels)
static BitmapT* prev_states[DISP_DEPTH];

// states_head % DISP_DEPTH points to the newest game state in prev_states
static u_short states_head = 0;

// phase (0-8) of blitter calculations
static u_short phase = 0; 

// magic constants for blitter operations
static u_short minterms_table[9] = {0x96, 0xE8, 0x7E, 0x69, 0x69, 0x81, 0xB4, 0x3A, 0x24};

static PaletteT palette = {
  .count = 16,
  .colors = {
    0x000,  // 0000
    0x006,  // 0001
    0x026,  // 0010
    0x026,  // 0011
    0x05B,  // 0100
    0x05B,  // 0101
    0x05B,  // 0110
    0x05B,  // 0111
    0x09F,  // 1000
    0x09F,  // 1001
    0x09F,  // 1010
    0x09F,  // 1011
    0x09F,  // 1100
    0x09F,  // 1101
    0x09F,  // 1110
    0x09F,  // 1111
  }
};

// setup blitter to calculate a function of three horizontally adjacent lit pixels
static void BlitAdjacentHorizontal(const BitmapT* source, BitmapT* target, u_short minterms)
{
  void* srcCenter = source->planes[0] + source->bytesPerRow; // C channel
  void* srcRight = source->planes[0] + source->bytesPerRow + 2; // B channel
  void* srcLeft = source->planes[0] + source->bytesPerRow; // A channel
  u_short bltsize = (BOARD_HEIGHT << 6) | ((EXT_BOARD_WIDTH / 16));

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

// setup blitter for a standard blit without shifts
static void BlitFunc(const BitmapT* sourceA, const BitmapT* sourceB, const BitmapT* sourceC, const BitmapT* target, u_short minterms)
{
  u_short bltsize = (BOARD_HEIGHT << 6) | ((EXT_BOARD_WIDTH / 16));

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

static void (*PixelDouble)(u_char* source asm("a0"), u_short* target asm("a1"), u_short* lut asm("a2"));
#define PixelDoubleSize (BOARD_WIDTH * BOARD_HEIGHT * 16 + 2)

// doubles pixels horizontally
static void MakePixelDoublingCode(const BitmapT* bitmap)
{
  u_short x;
  u_short y;
  u_short *code = (void*)PixelDouble;

  *code++ = 0x7200; // moveq #0,d1
  for (y = 1; y < bitmap->height - 1; y++)
  {
    for (x = 2; x < bitmap->bytesPerRow - 2; x++)
    {
      *code++ = 0x1218; // move.b (a0)+,d1
      *code++ = 0x2001; // move.l d1,d0
      *code++ = 0xd081; // add.l d1,d0
      *code++ = 0x32f2; //
      *code++ = 0x0800; // move.w (0,a2,d0.l),(a1)+
      // perform a lookup in the pixel doubling lookup table (e.g. 00100110 -> 0000110000111100)    	
      // *double_target++ = double_pixels[*double_src++];
    }
    *code++ = 0x5888; // addq.l #4,a0
    // double_src += 4;
    // bitmap modulo - skip the extra 4 bytes on the edges (2 on the left, 2 on the right on the next row)
  }
  *code++ = 0x4e75; // rts
}

static void MakeCopperList(CopListT* cp) {
  u_short i;

  CopInit(cp);
  // initially previous states are empty
  // save addresses of these instructions to change bitplane
  // order when new state gets generated
  bplptr[0] = CopMove32(cp, bplpt[0], prev_states[0]->planes[0]);
  bplptr[1] = CopMove32(cp, bplpt[1], prev_states[1]->planes[0]);
  bplptr[2] = CopMove32(cp, bplpt[2], prev_states[2]->planes[0]);
  bplptr[3] = CopMove32(cp, bplpt[3], prev_states[3]->planes[0]);
  for (i = 1; i <= DISP_HEIGHT; i += 2)
  {
    // vertical pixel doubling
    CopMove16(cp, bpl1mod, -prev_states[0]->bytesPerRow);
    CopMove16(cp, bpl2mod, -prev_states[0]->bytesPerRow);
    CopWaitSafe(cp, Y(i), 0);
    CopMove16(cp, bpl1mod, 0);
    CopMove16(cp, bpl2mod, 0);
    CopWaitSafe(cp, Y(i+1), 0);
  }
  CopEnd(cp);
}

static void UpdateBitplanePointers(void)
{
  BitmapT* cur;
  u_short i;
  for (i = 0; i < DISP_DEPTH; i++)
  {
    // update bitplane order: (states_head + i + 1) % DISP_DEPTH iterates from
    // the oldest to newest game state, so 0th bitplane displays the oldest state
    // and (DISP_DEPTH-1)'th bitplane displays the newest state
    cur = prev_states[(states_head + i + 1) % DISP_DEPTH];
    CopInsSet32(bplptr[i], cur->planes[0]);
  }
}

static void GameOfLife(void)
{
  ClearIRQ(INTF_BLIT);
  switch (phase)
  {
    // sum horizontally adjacent pixels - produces two results (sum bits): lo and hi
    case 0: BlitAdjacentHorizontal(&current_board, lo, minterms_table[0]); break;
    case 1: BlitAdjacentHorizontal(&current_board, hi, minterms_table[1]); break;

    // result based on the number of set bits in lo
    // set bits -> result
    // 0        -> 0
    // 1        -> 1
    // 2        -> 1
    // 3        -> 0
    case 2: BlitAdjacentVertical(lo, x0, minterms_table[2]); break;
    // set bits -> result
    // 0        -> 1
    // 1        -> 0
    // 2        -> 1
    // 3        -> 0
    case 3: BlitAdjacentVertical(lo, x1, minterms_table[3]); break;

    // result based on the number of set bits in hi
    // set bits -> result
    // 0 -> 1
    // 1 -> 0
    // 2 -> 1
    // 3 -> 0
    case 4: BlitAdjacentVertical(hi, x2, minterms_table[4]); break;
    // set bits -> result
    // 0 -> 1
    // 1 -> 0
    // 2 -> 0
    // 3 -> 1
    case 5: BlitAdjacentVertical(hi, x3, minterms_table[5]); break;

    // black magic happens here - combines previous results and initial game state into new state
    case 6: BlitFunc(x2, &current_board, x3, x5, minterms_table[6]); break;
    case 7: BlitFunc(x1, x5, x3, x6, minterms_table[7]); break;
    case 8: BlitFunc(x2, x0, x6, &current_board, minterms_table[8]); break;

    // hack - avoid graphical artifacts
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
  MakePixelDoublingCode(&current_board);

  BitmapClear(&current_board);
  BitmapCopy(&current_board, START_OFFSET_X+2, START_OFFSET_Y+10, &p46basedprng);

  cp = NewCopList(300);
  MakeCopperList(cp);
  CopListActivate(cp);

  EnableDMA(DMAF_RASTER);

  SetIntVector(BLIT, (IntHandlerT)GameOfLife, NULL);
  EnableINT(INTF_BLIT);
}

static void Kill(void) {
  u_short i;
  DisableDMA(DMAF_COPPER | DMAF_RASTER | DMAF_BLITTER);
  DisableINT(INTF_BLIT);
  ResetIntVector(BLIT);

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

  MemFree(PixelDouble);
  DeleteCopList(cp);
}

PROFILE(GOLStep)

static void Render(void) {
  ProfilerStart(GOLStep);
    PixelDouble(current_board.planes[0] + current_board.bytesPerRow + 2, prev_states[states_head % DISP_DEPTH]->planes[0], double_pixels);
    UpdateBitplanePointers();
    states_head++;
    phase = 0;
    GameOfLife();
  ProfilerStop(GOLStep);
}

EFFECT(game_of_life, NULL, NULL, Init, Kill, Render);
