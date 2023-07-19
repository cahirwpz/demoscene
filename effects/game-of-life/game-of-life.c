#include "bitmap.h"
#include "blitter.h"
#include "copper.h"
#include "effect.h"
#include "fx.h"
#include "sprite.h"
#include "pixmap.h"
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

// max amount boards for current board + intermediate results
#define BOARD_COUNT 12

#define DISP_WIDTH 320
#define DISP_HEIGHT 256
#define DISP_DEPTH 4
#define PREV_STATES_DEPTH (DISP_DEPTH + 1)
#define COLORS 16

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

//#define DEBUG_KBD

#ifdef DEBUG_KBD
  #include <system/event.h>
  #include <system/keyboard.h>
#endif

#include "data/p46basedprng.c"
#include "data/electric-lifeforms.c"
#include "data/cell-gradient.c"
#include "data/logo-electrons.c"
#include "games.c"

static CopListT *cp;
static BitmapT *current_board;

// current board = boards[0], the rest is intermediate results
static BitmapT *boards[BOARD_COUNT];

// pointers to copper instructions, for rewriting bitplane pointers
static CopInsPairT *bplptr;

// pointers to copper instructions, for setting colors
static CopInsT *palptr;

// circular buffer of previous game states as they would be rendered (with
// horizontally doubled pixels)
static BitmapT *prev_states[PREV_STATES_DEPTH];

// states_head % PREV_STATES_DEPTH points to the newest (currently being
// pixel-doubled, not displayed yet) game state in prev_states
static u_short states_head = 0;

// phase (0-8) of blitter calculations
static u_short phase = 0;

// like frameCount, but counts game of life generations (sim steps)
static u_short stepCount = 0;

// which wireworld game (0-1) phase are we on
static u_short wireworld_step = 0;

static const GameDefinitionT* current_game;

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
static void BlitAdjacentHorizontal(const BitmapT *sourceA, __attribute__((unused)) const BitmapT* sourceB,
                                   __attribute__((unused)) const BitmapT *sourceC, const BitmapT *target,
                                   u_short minterms) {
  void *srcCenter = sourceA->planes[0] + sourceA->bytesPerRow;    // C channel
  void *srcRight = sourceA->planes[0] + sourceA->bytesPerRow + 2; // B channel
  void *srcLeft = sourceA->planes[0] + sourceA->bytesPerRow;      // A channel
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
static void BlitAdjacentVertical(const BitmapT *sourceA, __attribute__((unused)) const BitmapT* sourceB,
                                 __attribute__((unused)) const BitmapT *sourceC, const BitmapT *target,
                                 u_short minterms) {
  void *srcCenter = sourceA->planes[0] + sourceA->bytesPerRow;   // C channel
  void *srcUp = sourceA->planes[0];                             // A channel
  void *srcDown = sourceA->planes[0] + 2 * sourceA->bytesPerRow; // B channel
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

// This is a bit hacky way to reuse the current interface for blitter phases
// to do something after the last blit has been completed. For technical
// reasons wireworld implementation requires 2 separate games to be ran
// in alternating fashion, and the switch between them must be done precisely
// when the board for a game finishes being calculated or things will break
// in ways undebuggable by single-stepping the game state
static void WireworldSwitch(__attribute__((unused)) const BitmapT *sourceA,
                            __attribute__((unused)) const BitmapT *sourceB,
                            __attribute__((unused)) const BitmapT *sourceC,
                            __attribute__((unused)) const BitmapT *target,
                            __attribute__((unused)) u_short minterms) {
  current_board = boards[wireworld_step];
  current_game = wireworlds[wireworld_step];
  wireworld_step ^= 1;
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

static void ChangePalette(const u_short* pal) {
    u_short i, j;
    
    // I wonder if this would be better just being unrolled...
    // increment `pal` on every power of 2, essentially setting
    // 4 consecutive colors from `pal` to 1, 2, 4 and 8 colors respectively
    for (i = 1; i < COLORS;)
    {
      u_short next_i = i << 1;
      for (j = i; j < next_i; j++)
        CopInsSet16(&palptr[j], *pal);
      i = next_i;
      pal++;
    }
}

static void MakeCopperList(CopListT *cp) {
  short i;

  CopInit(cp);
  // initially previous states are empty
  // save addresses of these instructions to change bitplane
  // order when new state gets generated
  bplptr = CopInsPtr(cp);
  for (i = 0; i < DISP_DEPTH; i++) 
    CopMove32(cp, bplpt[i], prev_states[i]->planes[0]);

  palptr = CopLoadPal(cp, &palette, 0);

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
    CopInsSet32(&bplptr[i - 1], cur->planes[0]);
  }
}

static void CyclePalette(void) {
  static u_short* cur_pal = gradient_pixels;
  static u_short pal_phase = 0;  

  ChangePalette(cur_pal);
  
  if (pal_phase)
    cur_pal -= 4;
  else
    cur_pal += 4;

  if (cur_pal >= (u_short*)(gradient_pixels)+gradient_width*gradient_height || cur_pal <= gradient_pixels) {
    pal_phase ^= 1;
    if (pal_phase)
      cur_pal -= 4;
    else
      cur_pal += 4;
  }
}

void GameOfLife(void) {
  ClearIRQ(INTF_BLIT);
  if (phase < current_game->num_phases) {
    const BlitterPhaseT *p = &current_game->phases[phase];
    p->blitfunc(boards[p->srca], boards[p->srcb], boards[p->srcc], boards[p->dst], p->minterm);
  }
  phase++;
}

static void SpawnElectrons(const ElectronsT *electrons, u_short board) {
  u_short i;
  for (i = 0; i < electrons->count; i++)
  {
    u_short bit = ((EXT_BOARD_WIDTH * (electrons->points[i].y + EXT_HEIGHT_TOP)) + 
                  (EXT_WIDTH_LEFT + electrons->points[i].x));
    u_short pos = bit / 8;
    bit = 7 - (bit & 0x7);
    bset((u_char*)boards[board]->planes[0]+pos, bit);
  }
}

static void Init(void) {
  u_short i;

  MakeDoublePixels();

  for (i = 0; i < BOARD_COUNT; i++)
    boards[i] = NewBitmap(EXT_BOARD_WIDTH, EXT_BOARD_HEIGHT, BOARD_DEPTH);

  current_board = boards[0];
  current_game = &wireworld1;
  // board 11 is special in case of wireworld - it contains the electron paths
  BitmapCopy(boards[11], EXT_WIDTH_LEFT, EXT_HEIGHT_TOP, &logo);

  SetupPlayfield(MODE_LORES, DISP_DEPTH, X(0), Y(0), DISP_WIDTH, DISP_HEIGHT);
  LoadPalette(&palette, 0);
  EnableDMA(DMAF_BLITTER);

  for (i = 0; i < PREV_STATES_DEPTH; i++) {
    // only needs half the vertical resolution, other half 
    // achieved via copper line doubling
    prev_states[i] = NewBitmap(DISP_WIDTH, DISP_HEIGHT / 2, BOARD_DEPTH);
    BitmapClear(prev_states[i]);
  }

  PixelDouble = MemAlloc(PixelDoubleSize, MEMF_PUBLIC);
  MakePixelDoublingCode(current_board);

   // electron heads/tails in case of wireworld
  BitmapClear(boards[0]);
  BitmapClear(boards[1]);

  // This took good part of my sanity do debug
  // Wait until the clearing is complete, otherwise we would overwrite
  // the bits we're setting below *after* they're set
  WaitBlitter();
  ClearIRQ(INTF_BLIT);

  cp = NewCopList(800);
  MakeCopperList(cp);
  CopListActivate(cp);

#ifdef DEBUG_KBD
  KeyboardInit();
#endif

  EnableDMA(DMAF_RASTER);

  SetIntVector(INTB_BLIT, (IntHandlerT)GameOfLife, NULL);
  EnableINT(INTF_BLIT);
}

static void Kill(void) {
  u_short i;

  DisableDMA(DMAF_RASTER | DMAF_BLITTER);
  DisableINT(INTF_BLIT);
  ResetIntVector(INTB_BLIT);

#ifdef DEBUG_KBD
  KeyboardKill();
#endif

  for (i = 0; i < BOARD_COUNT; i++)
    DeleteBitmap(boards[i]);

  for (i = 0; i < PREV_STATES_DEPTH; i++)
    DeleteBitmap(prev_states[i]);

  MemFree(PixelDouble);
  DeleteCopList(cp);
}

PROFILE(GOLStep)

static void GolStep(void) {
  void *dst = prev_states[states_head % PREV_STATES_DEPTH]->planes[0];
  void *src = current_board->planes[0] + current_board->bytesPerRow + EXT_WIDTH_LEFT / 8;

  ProfilerStart(GOLStep);
  PixelDouble(src, dst, double_pixels);
  UpdateBitplanePointers();
  states_head = (states_head+1) % PREV_STATES_DEPTH;
  phase = 0;
  GameOfLife();
  CyclePalette();
  
  if ((stepCount & 0x7) == 0) {
    // set pixels on correct board
    SpawnElectrons(&heads, wireworld_step^1);
    SpawnElectrons(&tails, wireworld_step);
  }

  stepCount++;

  ProfilerStop(GOLStep);
}

static u_short run_continous = 1;

#ifdef DEBUG_KBD

static bool HandleEvent(void) {
  EventT ev[1];

  if (!PopEvent(ev))
    return true;

  if (ev->type == EV_KEY && !(ev->key.modifier & MOD_PRESSED)) {
    if (ev->key.code == KEY_ESCAPE)
      return false;
    else if (ev->key.code == KEY_SPACE)
      GolStep();
    else if (ev->key.code == KEY_C)
      run_continous ^= 1;
  }

  return true;
}

#endif

static void Render(void) {
  if (run_continous)
    GolStep();

#ifdef DEBUG_KBD
  exitLoop = !HandleEvent();
#endif
}

EFFECT(GameOfLife, NULL, NULL, Init, Kill, Render, NULL);
