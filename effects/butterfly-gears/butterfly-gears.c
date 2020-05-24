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
#include "mouse.h"
#include "event.h"

#define DEPTH 5
#define BALLS 3
#define ROTZOOM_W 24
#define ROTZOOM_H 24
#define COPWAIT_X 1
#define Y0 Y((256-280)/2)
#define COPPER_HALFROW_INSTRUCTIONS (ROTZOOM_W/2+2)
#define INSTRUCTIONS_PER_BALL (COPPER_HALFROW_INSTRUCTIONS * ROTZOOM_H * 3)
#define DEBUG_COLOR_WRITES 0
#define MIN_ZOOM 2
#define MAX_ZOOM 80

#define SET_COLOR(x,rgb) CopMove16(cp, color[x], rgb)
#if DEBUG_COLOR_WRITES // only set background color for debugging
#define SET_TX_COLOR(x) CopMove16(cp, color[0], 0xf00)
#else
#define SET_TX_COLOR(x) CopMove16(cp, color[x], 0xf00)
#endif

// Internally, u/v coordinates use 8 fractional bits
#define f2short(f) \
  (short)((float)(f) * 256.0)

typedef struct {
  PixmapT texture;
  short angle;
  short angleDelta;
  short zoom;
  short zoomDelta;
  short u;
  short v;
  short uDelta;
  short vDelta;
  short screenX;
  short screenY;
  short screenLineHeight;
} BallT;

typedef struct {
  CopInsT *waitBefore;
  CopInsT *waitAfter;
  CopInsT *ballCopper;
  CopInsT *bplptr[DEPTH];
  CopInsT *bplcon1ins;
} BallCopInsertsT;

typedef struct {
  CopListT *cp;
  BallCopInsertsT inserts[BALLS];
} BallCopListT;

static BallCopListT ballCopList1; // TODO use second copper list and double-buffer
static BallT ball1;
static BallT ball2;
static BallT ball3;

#include "data/texture_butterfly.c"
#include "data/ball_small.c"
#include "data/ball_large.c"

// Pack u/v values into a longword to be used by the inner loop.
static inline long uv(short u, short v) {
  int combined;
  combined = (u & 0xffff) | ((v & 0xffff) << 16);
  return combined;
}

extern void PlotTextureAsm(char *copperDst asm("a0"),
                           char *texture   asm("a1"),
                           int  uvPosition asm("d5"),
                           int  uvDeltaRow asm("d6"),
                           int  uvDeltaCol asm("d1"));

// Create copper writes to color registers, leave out colors needed for sprites
static void InitCopperListBall(CopListT *cp, int y, int yInc) {
  short i;

  for (i=0; i<ROTZOOM_H; i++) {
    CopWait(cp, y, COPWAIT_X);
    SET_TX_COLOR(3);
    SET_TX_COLOR(5);
    SET_TX_COLOR(7);
    SET_TX_COLOR(9);
    SET_TX_COLOR(11);
    SET_TX_COLOR(13);
    SET_TX_COLOR(15);
    SET_TX_COLOR(18);
    SET_TX_COLOR(20);
    SET_TX_COLOR(23);
    SET_TX_COLOR(27);
    SET_TX_COLOR(31);
    CopNoOp(cp);
    y += yInc;
    CopWait(cp, y, COPWAIT_X);
    SET_TX_COLOR(2);
    SET_TX_COLOR(4);
    SET_TX_COLOR(6);
    SET_TX_COLOR(8);
    SET_TX_COLOR(10);
    SET_TX_COLOR(12);
    SET_TX_COLOR(14);
    SET_TX_COLOR(16);
    SET_TX_COLOR(19);
    SET_TX_COLOR(22);
    SET_TX_COLOR(24);
    SET_TX_COLOR(28);
    CopNoOp(cp);
    y += yInc;
  }
}

static void MakeBallCopperList(BallCopListT *ballCp) {
  short i;

  CopListT *cp = NewCopList(INSTRUCTIONS_PER_BALL * 3 + 100);
  ballCp->cp = cp;
  CopInit(cp);
  CopSetupDisplayWindow(cp, MODE_LORES, X(0), Y0, 320, 280);
  CopMove16(cp, dmacon, DMAF_SETCLR | DMAF_RASTER);
  SET_COLOR(0, 0x134);
  SET_COLOR(1, 0x000);
  CopSetupBitplaneFetch(cp, MODE_LORES, X(0), ball_large.width);

  for (i = 0; i < 1; i++) {
    ballCp->inserts[i].waitBefore = CopWait(cp, Y0, COPWAIT_X);
    CopSetupBitplanes(cp, ballCp->inserts[i].bplptr, &ball_large, ball_large.depth);
    ballCp->inserts[i].bplcon1ins = CopMove16(cp, bplcon1, 0);
    CopSetupMode(cp, MODE_LORES, ball_large.depth);
    ballCp->inserts[i].ballCopper = cp->curr;
    InitCopperListBall(cp, Y0 + 6, 2);
    ballCp->inserts[i].waitAfter = CopWait(cp, Y0 + 112, COPWAIT_X);
    CopMove16(cp, bplcon0, BPLCON0_COLOR);
  }

  CopEnd(cp);
}

static void DrawCopperBall(BallT *ball, BallCopInsertsT inserts) {

  {
    short x = 263 - ball->screenX;
    short bplSkip = (x / 8) & 0xfe;
    short shift = 15 - (x & 0xf);
    CopInsSet32(inserts.bplptr[0], ball_large.planes[0]+bplSkip);
    CopInsSet32(inserts.bplptr[1], ball_large.planes[1]+bplSkip);
    CopInsSet32(inserts.bplptr[2], ball_large.planes[2]+bplSkip);
    CopInsSet32(inserts.bplptr[3], ball_large.planes[3]+bplSkip);
    CopInsSet32(inserts.bplptr[4], ball_large.planes[4]+bplSkip);
    CopInsSet16(inserts.bplcon1ins, (shift << 4) | shift);
  }

  {
    short i;
    short y = ball->screenY;
    short yInc = ball->screenLineHeight;
    CopInsT *textureCopper = inserts.ballCopper;
    inserts.waitBefore->wait.vp = y;
    y += 6;
    for (i = 0; i < ROTZOOM_H; i++) {
      textureCopper->wait.vp = y;
      textureCopper += ROTZOOM_W / 2 + 2;
      y += yInc;
      textureCopper->wait.vp = y;
      textureCopper += ROTZOOM_W / 2 + 2;
      y += yInc;
    }
  }

  {
    short sin, cos;
    int u, v, deltaCol, deltaRow, uvPos;
    sin = (ball->zoom*SIN(ball->angle)) >> 9;
    cos = (ball->zoom*COS(ball->angle)) >> 9;
    deltaCol = uv(sin, cos);
    deltaRow = uv(cos, -sin);
    u = ball->u - sin * (ROTZOOM_W / 2) - cos * (ROTZOOM_H / 2);
    v = ball->v - cos * (ROTZOOM_W / 2) + sin * (ROTZOOM_H / 2);
    uvPos = uv(u, v);
    PlotTextureAsm((char *) inserts.ballCopper, (char *) ball->texture.pixels, uvPos, deltaCol, deltaRow);
    ball->angle += ball->angleDelta;
    ball->u += ball->uDelta;
    ball->v += ball->vDelta;
    ball->zoom += ball->zoomDelta;
    if (ball->zoom < MIN_ZOOM || ball->zoom > MAX_ZOOM) {
      ball->zoomDelta = -ball->zoomDelta;
    }
  }
}

static void Init(void) {
  MouseInit(0, 0, 262, 256);

  MakeBallCopperList(&ballCopList1);
  CopListActivate(ballCopList1.cp);

  ball1.texture = texture_butterfly;
  ball1.screenLineHeight = 2;
  ball1.angleDelta = 25;
  ball1.u = f2short(64.0f);
  ball1.v = 0;
  ball1.vDelta = f2short(0.7f);
  ball1.zoom = MIN_ZOOM;
  ball1.zoomDelta = 1;

  ball2.texture = texture_butterfly;
  ball1.screenLineHeight = 2;
  ball2.angleDelta = 0;
  ball2.uDelta = f2short(0.5f);
  ball2.vDelta = f2short(-0.3f);
  ball2.zoom = MIN_ZOOM + (MAX_ZOOM - MIN_ZOOM) / 2;
  ball2.zoomDelta = -1;

  ball3.texture = texture_butterfly;
  ball1.screenLineHeight = 2;
  ball3.angleDelta = -27;
  ball3.zoom = MIN_ZOOM + (MAX_ZOOM - MIN_ZOOM) * 3 / 2;
  ball3.u = f2short(64.f);
  ball3.vDelta = f2short(-0.2f);
}

static void Kill(void) {
  MouseKill();
  DeleteCopList(ballCopList1.cp);
}

static void HandleEvent(void) {
  EventT ev[1];

  if (!PopEvent(ev))
    return;

  if (ev->type == EV_MOUSE) {
    ball1.screenX = ev->mouse.x;
    ball1.screenY = ev->mouse.y;
  }
}

static void Render(void) {
  HandleEvent();
  DrawCopperBall(&ball1, ballCopList1.inserts[0]);
  if (0) {
    DrawCopperBall(&ball2, ballCopList1.inserts[1]);
    DrawCopperBall(&ball3, ballCopList1.inserts[2]);
  }
  TaskWaitVBlank();
}

EffectT Effect = { NULL, NULL, Init, Kill, Render };
