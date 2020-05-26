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
#define WIDTH 320
#define SMALL_BALL_PADDING_TOP 5
#define SMALL_BALL_PADDING_BOTTOM 8
#define SMALL_BALL_Y_INC 1
#define SMALL_BALL_WIDTH 58
#define SMALL_BALL_HEIGHT 60
#define SMALL_BALL_CENTER ((WIDTH-SMALL_BALL_WIDTH)/2+16)
#define LARGE_BALL_PADDING_TOP 6
#define LARGE_BALL_PADDING_BOTTOM 10
#define LARGE_BALL_Y_INC 2
#define LARGE_BALL_WIDTH 110
#define LARGE_BALL_HEIGHT 112
#define LARGE_BALL_CENTER ((WIDTH-LARGE_BALL_WIDTH)/2+16)
#define ROTZOOM_W 24
#define ROTZOOM_H 24
#define COPWAIT_X 0
#define COPWAIT_X_BALLSTART 160
#define Y0 Y((256-280)/2)
#define BOOK_Y 256
#define COPPER_HALFROW_INSTRUCTIONS (ROTZOOM_W/2+2)
#define INSTRUCTIONS_PER_BALL (COPPER_HALFROW_INSTRUCTIONS * ROTZOOM_H * 3)
#define DEBUG_COLOR_WRITES 0
#define MIN_ZOOM 2
#define MAX_ZOOM 80
#define NO_OP 0x1fe

#if DEBUG_COLOR_WRITES // only set background color for debugging
#define SETCOLOR(x) CopMove16(cp, color[0], 0xf00)
#else
#define SETCOLOR(x) CopMove16(cp, color[x], 0xf00)
#endif
#define SETBG(vp, rgb) CopWait(cp, vp, 7); \
                       CopSetColor(cp, 0, rgb)


// Internally, u/v coordinates use 8 fractional bits
#define f2short(f) \
  (short)((float)(f) * 256.0)

typedef enum { SMALL_BALL, LARGE_BALL } BallTypeT;

typedef struct {
  PixmapT texture;
  BallTypeT type;
  short angle;
  short angleDelta;
  short zoom;
  short zoomDelta;
  int u;
  int v;
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

static int active = 0;
static BallCopListT ballCopList[2];
static CopListT *bottomCp;
static CopInsT *bottomCpStart;
static BallT ball1;
static BallT ball2;
static BallT ball3;

#include "data/texture_butterfly.c"
#include "data/ball_small.c"
#include "data/ball_large.c"
#include "data/book_bottom.c"

extern void PlotTextureAsm(char *copperDst asm("a0"),
                           char *texture   asm("a1"),
                           int  u          asm("d0"),
                           int  v          asm("d2"),
                           int  uDeltaCol  asm("d1"),
                           int  vDeltaCol  asm("d3"),
                           int  uDeltaRow  asm("d5"),
                           int  vDeltaRow  asm("d6"));

// Create copper writes to color registers, leave out colors needed for sprites
static void InsertTextureCopperWrites(CopListT *cp) {
  short i;
  for (i=0; i<ROTZOOM_H; i++) {
    CopWait(cp, 0, COPWAIT_X);
    SETCOLOR(3);
    SETCOLOR(5);
    SETCOLOR(7);
    SETCOLOR(9);
    SETCOLOR(11);
    SETCOLOR(13);
    SETCOLOR(15);
    SETCOLOR(18);
    SETCOLOR(20);
    SETCOLOR(23);
    SETCOLOR(27);
    SETCOLOR(31);
    CopNoOp(cp);
    CopWait(cp, 0, COPWAIT_X);
    SETCOLOR(2);
    SETCOLOR(4);
    SETCOLOR(6);
    SETCOLOR(8);
    SETCOLOR(10);
    SETCOLOR(12);
    SETCOLOR(14);
    SETCOLOR(16);
    SETCOLOR(19);
    SETCOLOR(22);
    SETCOLOR(24);
    SETCOLOR(28);
    CopNoOpData(cp, i); // store row for copperlist debugging
  }
}

static void InitCopperList(BallCopListT *ballCp) {
  short i;

  CopListT *cp = NewCopList(INSTRUCTIONS_PER_BALL * 3 + 100);
  ballCp->cp = cp;
  CopInit(cp);
  CopSetupDisplayWindow(cp, MODE_LORES, X(0), Y0, 320, 280);
  CopMove16(cp, dmacon, DMAF_SETCLR | DMAF_RASTER);
  CopSetupMode(cp, MODE_LORES, 0);
  CopSetColor(cp, 0, 0x134);
  CopSetColor(cp, 1, 0x000);
  CopSetupBitplaneFetch(cp, MODE_LORES, X(0), ball_large.width);
  CopMove32(cp, cop2lc, bottomCpStart);
  CopMove32(cp, bplpt[0], ball_large.planes[0]);
  CopMove16(cp, bpl1mod, -WIDTH/8);
  CopMove16(cp, bpl2mod, -WIDTH/8);
  CopMove16(cp, bplcon0, BPLCON0_COLOR | BPLCON0_BPU(1));

  for (i = 0; i < BALLS; i++) {
    ballCp->inserts[i].waitBefore = CopWait(cp, Y0, COPWAIT_X_BALLSTART);
    CopMove16(cp, bplcon0, BPLCON0_COLOR | BPLCON0_BPU(0));
    CopSetupBitplanes(cp, ballCp->inserts[i].bplptr, &ball_large, ball_large.depth);
    ballCp->inserts[i].bplcon1ins = CopMove16(cp, bplcon1, 0);
    CopMove16(cp, bplcon0, BPLCON0_COLOR | BPLCON0_BPU(DEPTH));
    ballCp->inserts[i].ballCopper = cp->curr;
    InsertTextureCopperWrites(cp);
    ballCp->inserts[i].waitAfter = CopWait(cp, Y0 + 112, COPWAIT_X);
    CopMove16(cp, bplcon0, BPLCON0_COLOR | BPLCON0_BPU(1));
    CopMove16(cp, bpl1mod, -WIDTH/8);
    CopMove16(cp, bpl2mod, -WIDTH/8);
  }

  CopMove16(cp, copjmp2, 0);
}

static void InitBottomCopperList(CopListT *cp) {
  cp = NewCopList(80);
  CopInit(cp);
  bottomCpStart = CopNoOp(cp); // CopWaitSafe does not return the $FFDF wait instruction (if needed)
  CopWaitSafe(cp, BOOK_Y, 0);
  CopMove16(cp, bplcon0, BPLCON0_COLOR | BPLCON0_BPU(1));

  CopSetupBitplanes(cp, NULL, &book_bottom, book_bottom.depth);
  CopMove16(cp, bplcon0, BPLCON0_COLOR | BPLCON0_BPU(book_bottom.depth));
  CopMove16(cp, bplcon1, 0);

  CopLoadPal(cp, &book_pal, 0);
  SETBG(BOOK_Y + 5,  0x133);
  SETBG(BOOK_Y + 10, 0x124);
  SETBG(BOOK_Y + 15, 0x123);
  SETBG(BOOK_Y + 20, 0x122);
  SETBG(BOOK_Y + 25, 0x112);
  SETBG(BOOK_Y + 30, 0x012);
  SETBG(BOOK_Y + 35, 0x001);
  SETBG(BOOK_Y + 40, 0x002);
  CopWait(cp, BOOK_Y + book_bottom.height, COPWAIT_X);
  CopMove16(cp, bplcon0, BPLCON0_COLOR);
  CopEnd(cp);
}

static void DrawCopperBall(BallT *ball, BallCopInsertsT inserts) {
  bool small = ball->type == SMALL_BALL;

  // Ball out of sight (at bottom)?
  if (ball->screenY >= BOOK_Y - 1) {
    inserts.waitBefore->move.reg = CSREG(copjmp2);
    return;
  }
  inserts.waitBefore->move.reg = NO_OP;

  // Set X
  {
    short x = (small ? SMALL_BALL_CENTER : LARGE_BALL_CENTER) - ball->screenX;
    short bplSkip = (x / 8) & 0xfe;
    short shift = 15 - (x & 0xf);
    BitmapT bitmap = small ? ball_small : ball_large;
    if (ball->screenY <= Y0) {
      bplSkip += (Y0 - 1 - ball->screenY) * WIDTH / 8;
    }
    CopInsSet32(inserts.bplptr[0], bitmap.planes[0]+bplSkip);
    CopInsSet32(inserts.bplptr[1], bitmap.planes[1]+bplSkip);
    CopInsSet32(inserts.bplptr[2], bitmap.planes[2]+bplSkip);
    CopInsSet32(inserts.bplptr[3], bitmap.planes[3]+bplSkip);
    CopInsSet32(inserts.bplptr[4], bitmap.planes[4]+bplSkip);
    CopInsSet16(inserts.bplcon1ins, (shift << 4) | shift);
  }

  // Update copper waits according to Y
  {
    short i;
    short y = ball->screenY;
    short yInc = small ? SMALL_BALL_Y_INC : LARGE_BALL_Y_INC;
    CopInsT *textureCopper = inserts.ballCopper;
    if (y > Y0) {
      inserts.waitBefore->wait.vp = y;
    } else {
      inserts.waitBefore->wait.vp = 0;
    }
    inserts.waitBefore->wait.hp = COPWAIT_X_BALLSTART | 1;
    y += small ? SMALL_BALL_PADDING_TOP : LARGE_BALL_PADDING_TOP;
    for (i = 0; i < ROTZOOM_H * 2; i++) {
      if (y > Y0) {
        textureCopper->wait.vp = y;
      } else {
        textureCopper->wait.vp = 0;
      }
      textureCopper += ROTZOOM_W / 2 + 1;
      y += yInc;

      // Abort ball when the bottom book area starts
      if (y < BOOK_Y - 1) {
        textureCopper->move.reg = NO_OP;
      } else {
        textureCopper->move.reg = CSREG(copjmp2);
      }
      textureCopper++;
    }
    y += small ? SMALL_BALL_PADDING_BOTTOM : LARGE_BALL_PADDING_BOTTOM;
    if (y < BOOK_Y) {
      inserts.waitAfter->wait.vp = y;
      inserts.waitAfter->wait.hp = COPWAIT_X | 1;
    } else {
      inserts.waitAfter->wait.vp = 255;
      inserts.waitAfter->wait.hp = COPWAIT_X | 1;
    }
  }

  // Paint texture
  {
    short sin, cos;
    int u, v;
    sin = (ball->zoom*SIN(ball->angle)) >> 9;
    cos = (ball->zoom*COS(ball->angle)) >> 9;
    u = ball->u - sin * (ROTZOOM_W / 2) - cos * (ROTZOOM_H / 2);
    v = ball->v - cos * (ROTZOOM_W / 2) + sin * (ROTZOOM_H / 2);
    PlotTextureAsm((char *) inserts.ballCopper, (char *) ball->texture.pixels, u, v, sin, cos, cos, -sin);
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
  MouseInit(-100, -200, 100, 256);

  InitBottomCopperList(bottomCp);
  InitCopperList(&ballCopList[0]);
  InitCopperList(&ballCopList[1]);

  ball1.type = LARGE_BALL;
  ball1.texture = texture_butterfly;
  ball1.angleDelta = 14;
  ball1.u = f2short(64.0f);
  ball1.v = 0;
  ball1.vDelta = f2short(1.2f);
  ball1.zoom = MIN_ZOOM + (MAX_ZOOM - MIN_ZOOM) * 3 / 2;
  ball1.zoomDelta = 1;

  ball2.type = SMALL_BALL;
  ball2.texture = texture_butterfly;
  ball2.angleDelta = 5;
  ball2.uDelta = f2short(0.5f);
  ball2.vDelta = f2short(-0.3f);
  ball2.zoom = MIN_ZOOM + (MAX_ZOOM - MIN_ZOOM) / 2;
  ball2.zoomDelta = -1;

  ball3.type = SMALL_BALL;
  ball3.texture = texture_butterfly;
  ball3.angleDelta = -27;
  ball3.zoom = MIN_ZOOM + (MAX_ZOOM - MIN_ZOOM) * 3 / 2;
  ball3.u = f2short(64.f);
  ball3.vDelta = f2short(-0.2f);

  custom->dmacon = DMAF_MASTER | DMAF_COPPER | DMAF_SETCLR;
}

static void Kill(void) {
  MouseKill();
  DeleteCopList(ballCopList[0].cp);
  DeleteCopList(ballCopList[1].cp);
  DeleteCopList(bottomCp);
}

static void HandleEvent(void) {
  EventT ev[1];

  if (!PopEvent(ev))
    return;

  if (ev->type == EV_MOUSE) {
    ball1.screenX = ev->mouse.x;
    ball1.screenY = ev->mouse.y;
    ball2.screenY = ball1.screenY + LARGE_BALL_HEIGHT + 1;
    ball3.screenY = ball2.screenY + SMALL_BALL_HEIGHT + 1;
  }
}

static void Render(void) {
  HandleEvent();
  DrawCopperBall(&ball1, ballCopList[active].inserts[0]);
  DrawCopperBall(&ball2, ballCopList[active].inserts[1]);
  DrawCopperBall(&ball3, ballCopList[active].inserts[2]);
  TaskWaitVBlank();
  active ^= 1;
  CopListRun(ballCopList[active].cp);
}

EffectT Effect = { NULL, NULL, Init, Kill, Render };
