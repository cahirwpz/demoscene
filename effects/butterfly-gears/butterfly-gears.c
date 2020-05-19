#include "startup.h"

#include "hardware.h"
#include "interrupts.h"
#include "blitter.h"
#include "color.h"
#include "copper.h"
#include "bitmap.h"
#include "palette.h"
#include "pixmap.h"
#include "memory.h"
#include "sprite.h"
#include "fx.h"

#define ROTZOOM_W 24
#define ROTZOOM_H 24
#define COPWAIT_X 1
#define Y0 Y((256-280)/2)
#define COPPER_HALFROW_INSTRUCTIONS ROTZOOM_W/2+2
#define INSTRUCTIONS_PER_BALL COPPER_HALFROW_INSTRUCTIONS*2*ROTZOOM_H
#define DEBUG_COLOR_WRITES 0
#define USE_DEBUG_BITMAP 0

#if DEBUG_COLOR_WRITES // only set background color for debugging
#define COLREG_MASK 0
#else
#define COLREG_MASK 0xff
#endif

typedef struct {
  PixmapT texture;
  short angle;
  short angleDelta;
  int uvPos; // TODO handle u,v separately
  int uvDelta;
} BallT;

typedef struct {
  CopListT *cp;
  CopInsT *upperBallCopper;
  CopInsT *lowerBallCopper;
} BallCopListT;

static BallCopListT ballCopList1; // TODO use second copper list and double-buffer
static BallT ball1;
static BallT ball2;

#if USE_DEBUG_BITMAP
#include "data/gears_testscreen_debug.c"
#else
#include "data/gears_testscreen.c"
#endif
#include "data/texture_butterfly.c"
#include "data/texture_butterfly2.c"

// Create copper writes to color registers, leave out colors needed for sprites
static void InitCopperListBall(CopListT *cp, int y) {
  short i;

  for (i=0; i<ROTZOOM_H; i++) {
    CopWait(cp, y, COPWAIT_X);
    CopMove16(cp, color[ 3 & COLREG_MASK], 0xf00);
    CopMove16(cp, color[ 5 & COLREG_MASK], 0xf00);
    CopMove16(cp, color[ 7 & COLREG_MASK], 0xf00);
    CopMove16(cp, color[ 9 & COLREG_MASK], 0xf00);
    CopMove16(cp, color[11 & COLREG_MASK], 0xf00);
    CopMove16(cp, color[13 & COLREG_MASK], 0xf00);
    CopMove16(cp, color[15 & COLREG_MASK], 0xf00);
    CopMove16(cp, color[18 & COLREG_MASK], 0xf00);
    CopMove16(cp, color[20 & COLREG_MASK], 0xf00);
    CopMove16(cp, color[23 & COLREG_MASK], 0xf00);
    CopMove16(cp, color[26 & COLREG_MASK], 0xf00);
    CopMove16(cp, color[28 & COLREG_MASK], 0xf00);
    CopMoveWord(cp, 0x1fe, 0);
    y += 2;
    CopWait(cp, y, COPWAIT_X);
    CopMove16(cp, color[ 2 & COLREG_MASK], 0xf00);
    CopMove16(cp, color[ 4 & COLREG_MASK], 0xf00);
    CopMove16(cp, color[ 6 & COLREG_MASK], 0xf00);
    CopMove16(cp, color[ 8 & COLREG_MASK], 0xf00);
    CopMove16(cp, color[10 & COLREG_MASK], 0xf00);
    CopMove16(cp, color[12 & COLREG_MASK], 0xf00);
    CopMove16(cp, color[14 & COLREG_MASK], 0xf00);
    CopMove16(cp, color[16 & COLREG_MASK], 0xf00);
    CopMove16(cp, color[19 & COLREG_MASK], 0xf00);
    CopMove16(cp, color[22 & COLREG_MASK], 0xf00);
    CopMove16(cp, color[24 & COLREG_MASK], 0xf00);
    CopMove16(cp, color[27 & COLREG_MASK], 0xf00);
    CopMoveWord(cp, 0x1fe, 0);
    y += 2;
  }
}

static void MakeBallCopperList(BallCopListT *ballCp) {
  CopListT *cp = NewCopList(INSTRUCTIONS_PER_BALL * 2 + 100);
  ballCp->cp = cp;
  CopInit(cp);
  CopSetupDisplayWindow(cp, MODE_LORES, X(0), Y0, 320, 280);
  CopMove16(cp, dmacon, DMAF_SETCLR | DMAF_RASTER);
  CopLoadPal(cp, &testscreen_pal, 0);
  CopSetupMode(cp, MODE_LORES, testscreen.depth);
  CopSetupBitplanes(cp, NULL, &testscreen, testscreen.depth);
  CopSetupBitplaneFetch(cp, MODE_LORES, X(0), testscreen.width);

  ballCp->upperBallCopper = cp->curr;
  InitCopperListBall(cp, Y0 + 7);

  ballCp->lowerBallCopper = cp->curr;
  InitCopperListBall(cp, Y0 + 127);

  CopEnd(cp);
}

static void Init(void) {
  MakeBallCopperList(&ballCopList1);
  CopListActivate(ballCopList1.cp);
  ball1.texture = texture_butterfly;
  ball1.angleDelta = 27;
  ball1.uvDelta = 1 << 25;
  ball2.texture = texture_butterfly2;
  ball2.angleDelta = -11;
  ball2.uvDelta = -3 << (6+16);
}

static void Kill(void) {
  DeleteCopList(ballCopList1.cp);
}

extern void PlotTextureAsm(char *copperDst asm("a0"),
                           char *texture   asm("a1"),
                           int  uvPosition asm("d5"),
                           int  uvDeltaRow asm("d6"),
                           int  uvDeltaCol asm("d1"));

// Pack texture coordinates into a longword. Format:
// Bits = UUUUUUUu uuuuuuuu VVVVVVVv vvvvvvvv
// i.e. 7 integer bits and 9 fractional bits each
static uint32_t uv(short u, short v) {
  int combined;
  combined = (v & 0xffff) | ((u & 0xffff) << 16);
  return combined;
}

static void DrawCopperBall(CopInsT *copper, BallT *ball) {
  short sin;
  short cos;
  int pos;
  int deltaCol;
  int deltaRow;

  sin = SIN(ball->angle) >> 3;
  cos = COS(ball->angle) >> 3;
  deltaCol = uv(sin, cos);
  deltaRow = uv(cos, -sin);
  pos = -deltaCol * (ROTZOOM_W / 2) - deltaRow * (ROTZOOM_H / 2) + ball->uvPos;
  PlotTextureAsm((char*) copper, (char*) ball->texture.pixels, pos, deltaCol, deltaRow);
  ball->angle += ball->angleDelta;
  ball->uvPos += ball->uvDelta;
}

static void Render(void) {
  DrawCopperBall(ballCopList1.upperBallCopper, &ball1);
  DrawCopperBall(ballCopList1.lowerBallCopper, &ball2);
  TaskWaitVBlank();
}

EffectT Effect = { NULL, NULL, Init, Kill, Render };
