#include "bitmap.h"
#include "blitter.h"
#include "copper.h"
#include "effect.h"
#include "fx.h"
#include "sprite.h"
#include <system/interrupt.h>
#include <system/memory.h>

// This effect calculates Conway's game of life (with the classic rules: live
// cells with <2 or >3 neighbours die, live cells with 2-3 neighbouring cells
// live to the next generation and dead cells with 3 neighbours become alive).
// If you need more introductory information check out
// https://en.wikipedia.org/wiki/Conway's_Game_of_Life
//
// It works the following way - on each iteration we start with a board (a 1-bit
// bitmap) that is the current game state. On this bitmap, 1's represent alive
// cells, 0's represent dead cells. We perform several blits: first on this
// bitmap, producing some intermediate results and then on those intermediate
// results as well, extensively making use of the blitter's function generator.
// Minterms for the function generator, the order of blits and input sources
// were picked in such a way, that the end result calculates the next board
// state. Then the board gets its pixels horizontally and vertically doubled -
// horizontally by the CPU (using lookup table), vertically by the copper (by
// line doubling).  Previously calculated game states (with pixels already
// doubled) are kept in a circular buffer and displayed on separate bitplanes
// with dimmer colors (the dimmer the color, the more time has passed since cell
// on that square died). This process is repeated indefinitely.

#define DISP_WIDTH 320
#define DISP_HEIGHT 256
#define DISP_DEPTH 4
#define PREV_STATES_DEPTH (DISP_DEPTH + 1)

#define EXT_WIDTH_LEFT 16
#define EXT_WIDTH_RIGHT 16
#define EXT_HEIGHT_TOP 1
#define EXT_HEIGHT_BOTTOM 1
#define EXT_BOARD_MODULO (EXT_WIDTH_LEFT / 8 + EXT_WIDTH_RIGHT / 8)

#define BOARD_WIDTH (DISP_WIDTH / 2)
#define BOARD_HEIGHT (DISP_HEIGHT / 2)
#define EXT_BOARD_WIDTH (BOARD_WIDTH + EXT_WIDTH_LEFT + EXT_WIDTH_RIGHT)
#define EXT_BOARD_HEIGHT (BOARD_HEIGHT + EXT_HEIGHT_TOP + EXT_HEIGHT_BOTTOM)
#define BOARD_DEPTH 1

// "EXT_BOARD" is the area on which the game of life is calculated
// "BOARD" is the area which will actually be displayed (size before pixel
// doubling). Various constants are best described using a drawing (all shown
// constants are in pixels, drawing not to scale):
//
// -----------------------------------------------------------------------------
// |                                     ^                                     |
// |                                     | EXT_HEIGHT_TOP                      |
// |                                     v                                     |
// |                    -----------------------------------                    |
// |                    |           BOARD_WIDTH     ^     |                    |
// |                    |<--------------------------|---->|                    |
// |                    |                           |     |                    |
// |<------------------>|              BOARD_HEIGHT |     |<------------------>|
// |   EXT_WIDTH_LEFT   |                           |     |   EXT_WIDTH_RIGHT  |
// |                    |                           |     |                    |
// |                    |                           v     |                    |
// |                    -----------------------------------                    |
// |                                     ^                                     |
// |                                     | EXT_HEIGHT_BOTTOM                   |
// |                                     v                                     |
// -----------------------------------------------------------------------------
//

#include "data/p46basedprng.c"

static CopListT *cp;
static BitmapT *current_board;

// intermediate results
static BitmapT *lo;
static BitmapT *hi;
static BitmapT *x0;
static BitmapT *x1;
static BitmapT *x2;
static BitmapT *x3;
static BitmapT *x5;
static BitmapT *x6;

// pointers to copper instructions, for rewriting bitplane pointers
static CopInsT *bplptr[DISP_DEPTH];

// circular buffer of previous game states as they would be rendered (with
// horizontally doubled pixels)
static BitmapT *prev_states[PREV_STATES_DEPTH];

// states_head % PREV_STATES_DEPTH points to the newest (currently being
// pixel-doubled, not displayed yet) game state in prev_states
static u_short states_head = 0;

// phase (0-8) of blitter calculations
static u_short phase = 0;

// minterms for blitter operations
static u_short minterms_table[9] = {
  FULL_ADDER,
  FULL_ADDER_CARRY,
  NANBC | NABNC | NABC | ANBNC | ANBC | ABNC,
  NANBNC | NABC | ANBC | ABNC,
  NANBNC | NABC | ANBC | ABNC,
  NANBNC | ABC,
  NABNC | ANBNC | ANBC | ABC,
  NANBC | NABC | ANBNC | ANBC,
  NABNC | ANBC,
};

static PaletteT palette = {
  .count = 16,
  .colors =
    {
      0x000, // 0000
      0x006, // 0001
      0x026, // 0010
      0x026, // 0011
      0x05B, // 0100
      0x05B, // 0101
      0x05B, // 0110
      0x05B, // 0111
      0x09F, // 1000
      0x09F, // 1001
      0x09F, // 1010
      0x09F, // 1011
      0x09F, // 1100
      0x09F, // 1101
      0x09F, // 1110
      0x09F, // 1111
    },
};

// Used by CPU to quickly transform 1x1 pixels into 2x1 pixels.
static u_short double_pixels[256];

static void MakeDoublePixels(void) {
  u_short *data = double_pixels;
  u_short w = 0;
  short i;

  for (i = 0; i < 256; i++) {
    *data++ = w;
    w |= 0xAAAA; /* let carry propagate through odd bits */
    w++;
    w &= 0x5555;
    w |= w << 1;
  }
}

// setup blitter to calculate a function of three horizontally adjacent lit
// pixels the setup in this blit is as follows (what data each channel sees):
//            -1                 0                 1                 2
// C: [---------------] [---------------] [c--------------] [---------------]
//            -1                 0                 1                 2
// A: [---------------] [--------------a] [a--------------] [---------------]
//                                     |   ^
//                                     >>>>^
//                         'a' gets shifted one to the right
//
//             0                 1                 2                 3
// B: [---------------] [-b-------------] [b--------------] [---------------]
//                        |                ^
//                        >>>>>>>>>>>>>>>>>^
//                'b' starts one word later and gets shifted 15 to the right
//
// Thus a, b and c are lined up properly to perform boolean function on them
//
static void BlitAdjacentHorizontal(const BitmapT *source, BitmapT *target,
                                   u_short minterms) {
  void *srcCenter = source->planes[0] + source->bytesPerRow;    // C channel
  void *srcRight = source->planes[0] + source->bytesPerRow + 2; // B channel
  void *srcLeft = source->planes[0] + source->bytesPerRow;      // A channel
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
static void BlitAdjacentVertical(const BitmapT *source, BitmapT *target,
                                 u_short minterms) {
  void *srcCenter = source->planes[0] + source->bytesPerRow;   // C channel
  void *srcUp = source->planes[0];                             // A channel
  void *srcDown = source->planes[0] + 2 * source->bytesPerRow; // B channel
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
static void BlitFunc(const BitmapT *sourceA, const BitmapT *sourceB,
                     const BitmapT *sourceC, const BitmapT *target,
                     u_short minterms) {
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

static void (*PixelDouble)(u_char *source asm("a0"), u_short *target asm("a1"),
                           u_short *lut asm("a2"));

#define PixelDoubleSize (BOARD_WIDTH * BOARD_HEIGHT * 10 + BOARD_HEIGHT * 2 + 6)

// doubles pixels horizontally
static void MakePixelDoublingCode(const BitmapT *bitmap) {
  u_short x;
  u_short y;
  u_short *code = (void *)PixelDouble;

  *code++ = 0x7200 | (EXT_BOARD_MODULO & 0xFF); // moveq #EXT_BOARD_MODULO,d1
  for (y = EXT_HEIGHT_TOP; y < bitmap->height - EXT_HEIGHT_BOTTOM; y++) {
    for (x = EXT_WIDTH_LEFT / 8; x < bitmap->bytesPerRow - EXT_WIDTH_RIGHT / 8;
         x++) {
      *code++ = 0x4240; // clr.w  d0               # 4
      *code++ = 0x1018; // move.b (a0)+,d0         # 8
      *code++ = 0xd080; // add.l  d0,d0            # 6
      *code++ = 0x32f2;
      *code++ = 0x0000; // move.w (a2,d0.w),(a1)+  # 18
      // perform a lookup in the pixel doubling lookup table
      // (e.g. 00100110 -> 0000110000111100)
      // *double_target++ = double_pixels[*double_src++];
    }
    *code++ = 0xD1C1; // adda.l d1,a0
    // double_src += EXT_BOARD_MODULO & 0xFF;
    // bitmap modulo - skip the extra EXT_BOARD_MODULO bytes on the edges
    // (EXT_WIDTH_LEFT/8 bytes on the left, EXT_WIDTH_RIGHT/8 bytes on the right
    // on the next row)
  }
  *code++ = 0x4e75; // rts
}

static void MakeCopperList(CopListT *cp) {
  u_short i;

  CopInit(cp);
  // initially previous states are empty
  // save addresses of these instructions to change bitplane
  // order when new state gets generated
  bplptr[0] = CopMove32(cp, bplpt[0], prev_states[0]->planes[0]);
  bplptr[1] = CopMove32(cp, bplpt[1], prev_states[1]->planes[0]);
  bplptr[2] = CopMove32(cp, bplpt[2], prev_states[2]->planes[0]);
  bplptr[3] = CopMove32(cp, bplpt[3], prev_states[3]->planes[0]);
  for (i = 1; i <= DISP_HEIGHT; i += 2) {
    // vertical pixel doubling
    CopMove16(cp, bpl1mod, -prev_states[0]->bytesPerRow);
    CopMove16(cp, bpl2mod, -prev_states[0]->bytesPerRow);
    CopWaitSafe(cp, Y(i), 0);
    CopMove16(cp, bpl1mod, 0);
    CopMove16(cp, bpl2mod, 0);
    CopWaitSafe(cp, Y(i + 1), 0);
  }
  CopEnd(cp);
}

static void UpdateBitplanePointers(void) {
  BitmapT *cur;
  u_short i;
  for (i = 1; i < PREV_STATES_DEPTH; i++) {
    // update bitplane order: (states_head + i + 1) % PREV_STATES_DEPTH iterates
    // from the oldest+1 (to facilitate double buffering; truly oldest state is
    // the one we won't display as it's gonna be a buffer for the next game
    // state) to newest game state, so 0th bitplane displays the oldest+1 state
    // and (PREV_STATES_DEPTH-1)'th bitplane displays the newest state
    cur = prev_states[(states_head + i + 1) % PREV_STATES_DEPTH];
    CopInsSet32(bplptr[i - 1], cur->planes[0]);
  }
}

INTSERVER(RotateBitplanes, 0, (IntFuncT)UpdateBitplanePointers, NULL);

static void GameOfLife(void) {
  ClearIRQ(INTF_BLIT);

  switch (phase) {
    // sum horizontally adjacent pixels - produces two results (sum bits): lo
    // and hi
    case 0:
      BlitAdjacentHorizontal(current_board, lo, minterms_table[0]);
      break;
    case 1:
      BlitAdjacentHorizontal(current_board, hi, minterms_table[1]);
      break;

      // result based on the number of set bits in lo
      // set bits -> result
      // 0        -> 0
      // 1        -> 1
      // 2        -> 1
      // 3        -> 0
    case 2:
      BlitAdjacentVertical(lo, x0, minterms_table[2]);
      break;

      // set bits -> result
      // 0        -> 1
      // 1        -> 0
      // 2        -> 1
      // 3        -> 0
    case 3:
      BlitAdjacentVertical(lo, x1, minterms_table[3]);
      break;

      // result based on the number of set bits in hi
      // set bits -> result
      // 0 -> 1
      // 1 -> 0
      // 2 -> 1
      // 3 -> 0
    case 4:
      BlitAdjacentVertical(hi, x2, minterms_table[4]);
      break;

      // set bits -> result
      // 0 -> 1
      // 1 -> 0
      // 2 -> 0
      // 3 -> 1
    case 5:
      BlitAdjacentVertical(hi, x3, minterms_table[5]);
      break;

      // black magic happens here - combines previous results and initial game
      // state into new state
    case 6:
      BlitFunc(x2, current_board, x3, x5, minterms_table[6]);
      break;

    case 7:
      BlitFunc(x1, x5, x3, x6, minterms_table[7]);
      break;

    case 8:
      BlitFunc(x2, x0, x6, current_board, minterms_table[8]);
      break;
  }
  phase++;
}

static void Init(void) {
  u_short i;

  MakeDoublePixels();

  lo = NewBitmap(EXT_BOARD_WIDTH, EXT_BOARD_HEIGHT, BOARD_DEPTH);
  hi = NewBitmap(EXT_BOARD_WIDTH, EXT_BOARD_HEIGHT, BOARD_DEPTH);
  x0 = NewBitmap(EXT_BOARD_WIDTH, EXT_BOARD_HEIGHT, BOARD_DEPTH);
  x1 = NewBitmap(EXT_BOARD_WIDTH, EXT_BOARD_HEIGHT, BOARD_DEPTH);
  x2 = NewBitmap(EXT_BOARD_WIDTH, EXT_BOARD_HEIGHT, BOARD_DEPTH);
  x3 = NewBitmap(EXT_BOARD_WIDTH, EXT_BOARD_HEIGHT, BOARD_DEPTH);
  x5 = NewBitmap(EXT_BOARD_WIDTH, EXT_BOARD_HEIGHT, BOARD_DEPTH);
  x6 = NewBitmap(EXT_BOARD_WIDTH, EXT_BOARD_HEIGHT, BOARD_DEPTH);

  current_board = NewBitmap(EXT_BOARD_WIDTH, EXT_BOARD_HEIGHT, BOARD_DEPTH);

  SetupPlayfield(MODE_LORES, DISP_DEPTH, X(0), Y(0), DISP_WIDTH, DISP_HEIGHT);
  LoadPalette(&palette, 0);
  EnableDMA(DMAF_BLITTER);

  for (i = 0; i < PREV_STATES_DEPTH; i++) {
    prev_states[i] = NewBitmap(DISP_WIDTH, DISP_HEIGHT, BOARD_DEPTH);
    BitmapClear(prev_states[i]);
  }

  PixelDouble = MemAlloc(PixelDoubleSize, MEMF_PUBLIC);
  MakePixelDoublingCode(current_board);

  // set up initial state
  BitmapClear(current_board);
  BitmapCopy(current_board, 20, EXT_HEIGHT_TOP + 10, &p46basedprng);

  cp = NewCopList(800);
  MakeCopperList(cp);
  CopListActivate(cp);

  EnableDMA(DMAF_RASTER | DMAF_SPRITE);

  SetIntVector(INTB_BLIT, (IntHandlerT)GameOfLife, NULL);
  AddIntServer(INTB_VERTB, RotateBitplanes);
  EnableINT(INTF_BLIT);
}

static void Kill(void) {
  u_short i;

  DisableDMA(DMAF_COPPER | DMAF_RASTER | DMAF_BLITTER | DMAF_SPRITE);
  DisableINT(INTF_BLIT);
  ResetIntVector(INTB_BLIT);
  RemIntServer(INTB_VERTB, RotateBitplanes);

  DeleteBitmap(lo);
  DeleteBitmap(hi);
  DeleteBitmap(x0);
  DeleteBitmap(x1);
  DeleteBitmap(x2);
  DeleteBitmap(x3);
  DeleteBitmap(x5);
  DeleteBitmap(x6);
  DeleteBitmap(current_board);

  for (i = 0; i < PREV_STATES_DEPTH; i++)
    DeleteBitmap(prev_states[i]);

  MemFree(PixelDouble);
  DeleteCopList(cp);
}

PROFILE(GOLStep)

static void Render(void) {
  void *src = prev_states[states_head % PREV_STATES_DEPTH]->planes[0];
  void *dst =
    current_board->planes[0] + current_board->bytesPerRow + EXT_WIDTH_LEFT / 8;

  ProfilerStart(GOLStep);
  PixelDouble(dst, src, double_pixels);
  states_head++;
  phase = 0;
  GameOfLife();
  ProfilerStop(GOLStep);
}

EFFECT(GameOfLife, NULL, NULL, Init, Kill, Render);
