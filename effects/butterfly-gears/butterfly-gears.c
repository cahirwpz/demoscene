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
#include "random.h"

#define DEPTH 5
#define BALLS 3
#define WIDTH 320
#define BALL_PADDING_TOP 6
#define BALL_PADDING_BOTTOM 11
#define SMALL_BALL_Y_INC 1
#define SMALL_BALL_WIDTH 58
#define SMALL_BALL_HEIGHT 64
#define SMALL_BALL_CENTER ((WIDTH-SMALL_BALL_WIDTH)/2+16)
#define LARGE_BALL_Y_INC 2
#define LARGE_BALL_WIDTH 110
#define LARGE_BALL_HEIGHT 112
#define LARGE_BALL_CENTER ((WIDTH-LARGE_BALL_WIDTH)/2+16)
#define STATIC_Y_AREA_PADDING (LARGE_BALL_HEIGHT + 1)
#define ROTZOOM_W 24
#define ROTZOOM_H 24
#define COPWAIT_X 0
#define COPWAIT_X_BALLSTART 160
#define Y0 Y((256-280)/2)
#define STATIC_Y_AREA 40
#define BOOK_X(i) (X(94)+16*i)
#define BOOK_Y 256
#define STATIC_Y_START (BOOK_Y-STATIC_Y_AREA)
#define STATIC_Y_COMMANDS (STATIC_Y_AREA + STATIC_Y_AREA_PADDING * 2)
#define SPRITE_COORD_SHIFT 2
#define MIN_SPRITE_Y       (27 << SPRITE_COORD_SHIFT)
#define MAX_SPRITE_Y       ((BOOK_Y - 16 - 12) << SPRITE_COORD_SHIFT)
#define NEW_SPRITE_PADDING (32 << SPRITE_COORD_SHIFT)

// Copper texture layout. Goal: Have 1 spare copper move per screen row for
// sprite fades, bplcon2 at stable Y positions.
//
// Small ball (tx line every Y)     |  Large ball (tx line every 2 Y)
//
// W C C C C C C C C C C C C X N N  |  W C C C C C C C C C C C C X w X
// W C C C C C C C C C C C C X N N  |  W C C C C C C C C C C C C X w X
// W C C C C C C C C C C C C X N N  |  W C C C C C C C C C C C C X w X
// ...                                 ...
//
// W - wait
// w - extra wait between W and next W (Y and Y+2)
// C - set color
// X - spare copper move
// N - copper NO-OP

#define COPPER_HALFROW_INSTRUCTIONS (ROTZOOM_W/2+4)
#define INSTRUCTIONS_PER_BALL (COPPER_HALFROW_INSTRUCTIONS * ROTZOOM_H * 3 + 500) // ?!
#define DEBUG_COLOR_WRITES 0
#define ZOOM_SHIFT 4
#define NORM_ZOOM (1 << ZOOM_SHIFT)
#define MIN_ZOOM (NORM_ZOOM / 5)
#define MAX_ZOOM (NORM_ZOOM * 8)
#define NO_OP 0x1fe

#if DEBUG_COLOR_WRITES // only set background color for debugging
#define SETCOLOR(x) CopMove16(cp, color[0], 0x000)
#else
#define SETCOLOR(x) CopMove16(cp, color[x], 0x000)
#endif
#define SETBG(vp, rgb) CopWait(cp, vp, 7); \
                       CopSetColor(cp, 0, rgb)


// Internally, u/v coordinates use 8 fractional bits
#define f2short(f) \
  (short)((float)(f) * 256.0)

typedef struct {
  PixmapT texture;
  short height;
  short angle;
  short angleDelta;
  short zoom;
  short zoomSinPos;
  short zoomSinStep;
  short zoomSinAmp;
  int u;
  int v;
  short uDelta;
  short vDelta;
  short screenX;
  short screenY;
  short screenLineHeight;
} BallT;

typedef struct {
  CopInsT *copperJumpTarget;
  CopInsT *waitBefore;
  CopInsT *paddingTop;
  CopInsT *paddingBottom;
  CopInsT *ballCopper;
  CopInsT *bplptr[DEPTH];
  CopInsT *bplcon1ins;
} BallCopInsertsT;

typedef struct {
  CopListT *cp;
  BallCopInsertsT inserts[BALLS];
  CopInsT *staticAreaCopperStart;
  CopInsT *spritePointers;
} BallCopListT;

static int active = 0;
static BallCopListT ballCopList[2];
static CopListT *bottomCp;
static CopInsT *bottomCpStart;
static BallT ball1;
static BallT ball2;
static BallT ball3;
static BallT *balls[BALLS] = { &ball1, &ball2, &ball3 };
static bool mouseControlled = false;
static bool mouseMoved = false;
static CopInsT staticYCommands[STATIC_Y_COMMANDS];
static BallCopInsertsT *lastInsertsTop;
static BallCopInsertsT *lastInsertsBottom;

#include "data/texture_butterfly.c"
#include "data/ball_small.c"
#include "data/ball_large.c"
#include "data/book_bottom.c"
#include "data/sprites_book.c"
#include "data/spr0.c"
#include "data/spr1.c"
#include "data/spr2.c"
#include "data/spr3.c"
#include "data/spr4.c"
#include "data/spr5.c"
#include "data/spr6.c"
#include "data/spr7.c"

extern void PlotTextureAsm(char *copperDst asm("a0"),
                           char *texture   asm("a1"),
                           int  u          asm("d0"),
                           int  v          asm("d2"),
                           int  uDeltaCol  asm("d1"),
                           int  vDeltaCol  asm("d3"),
                           int  uDeltaRow  asm("d5"),
                           int  vDeltaRow  asm("d6"));

extern void CopyTextureAsm(char *copperSrc asm("a0"), char *copperDst asm("a1"));

extern void WriteStaticYArea(CopInsT *copperDst        asm("a0"),
                             CopInsT *copperSrc        asm("a1"),
                             int     bottomBallY       asm("d0"),
                             CopInsT *bottomBallCopper asm("a2"),
                             CopInsT *bookCopper       asm("a3"));

extern void UpdateBallCopper(int     y              asm("d0"),
                             int     yInc           asm("d1"),
                             CopInsT *staticYSource asm("a0"),
                             CopInsT *textureCopper asm("a1"),
                             CopInsT *paddingTop    asm("a2"),
                             CopInsT *paddingBottom asm("a3"));

extern void SetSprites(char    *sprite asm("a0"),
                       short   reg     asm("d0"),
                       short   xy[][]  asm("a2"));


static void InitStaticYCommands(void) {
  short i, idx;
  idx = 0;
  for (i = 0; i < STATIC_Y_AREA + STATIC_Y_AREA_PADDING * 2; i++) {
    staticYCommands[idx++] = (CopInsT) { .move = { .reg = NO_OP, .data = 0 } };
  }
  idx = STATIC_Y_AREA_PADDING + STATIC_Y_AREA - 6;
  staticYCommands[--idx] = (CopInsT) { .move = { .reg = CSREG(bplcon2), .data = 0x24 } };
  staticYCommands[--idx] = (CopInsT) { .move = { .reg = CSREG(color[17]), .data = 0x6cf } };
  staticYCommands[--idx] = (CopInsT) { .move = { .reg = CSREG(color[21]), .data = 0x6cf } };
  staticYCommands[--idx] = (CopInsT) { .move = { .reg = CSREG(color[18]), .data = 0x48a } };
  staticYCommands[--idx] = (CopInsT) { .move = { .reg = CSREG(color[22]), .data = 0x48a } };

  staticYCommands[--idx] = (CopInsT) { .move = { .reg = CSREG(color[17]), .data = 0x134 } };
  staticYCommands[--idx] = (CopInsT) { .move = { .reg = CSREG(color[21]), .data = 0x134 } };
  staticYCommands[--idx] = (CopInsT) { .move = { .reg = CSREG(color[18]), .data = 0x134 } };
  staticYCommands[--idx] = (CopInsT) { .move = { .reg = CSREG(color[22]), .data = 0x134 } };

  staticYCommands[--idx] = (CopInsT) { .move = { .reg = CSREG(color[17]), .data = 0x235 } };
  staticYCommands[--idx] = (CopInsT) { .move = { .reg = CSREG(color[21]), .data = 0x234 } };
  staticYCommands[--idx] = (CopInsT) { .move = { .reg = CSREG(color[18]), .data = 0x235 } };
  staticYCommands[--idx] = (CopInsT) { .move = { .reg = CSREG(color[22]), .data = 0x236 } };

  staticYCommands[--idx] = (CopInsT) { .move = { .reg = CSREG(color[17]), .data = 0x246 } };
  staticYCommands[--idx] = (CopInsT) { .move = { .reg = CSREG(color[21]), .data = 0x245 } };
  staticYCommands[--idx] = (CopInsT) { .move = { .reg = CSREG(color[18]), .data = 0x246 } };
  staticYCommands[--idx] = (CopInsT) { .move = { .reg = CSREG(color[22]), .data = 0x245 } };

  staticYCommands[--idx] = (CopInsT) { .move = { .reg = CSREG(color[17]), .data = 0x366 } };
  staticYCommands[--idx] = (CopInsT) { .move = { .reg = CSREG(color[21]), .data = 0x256 } };
  staticYCommands[--idx] = (CopInsT) { .move = { .reg = CSREG(color[18]), .data = 0x366 } };
  staticYCommands[--idx] = (CopInsT) { .move = { .reg = CSREG(color[22]), .data = 0x256 } };
}

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
    SETCOLOR(19);
    SETCOLOR(23);
    SETCOLOR(26);
    SETCOLOR(28);
    SETCOLOR(31);
    CopNoOpData(cp, i); // store row for copperlist debugging
    CopNoOpData(cp, i);
    CopNoOpData(cp, i);
    CopWait(cp, 0, COPWAIT_X);
    SETCOLOR(2);
    SETCOLOR(4);
    SETCOLOR(6);
    SETCOLOR(8);
    SETCOLOR(10);
    SETCOLOR(12);
    SETCOLOR(14);
    SETCOLOR(16);
    SETCOLOR(20);
    SETCOLOR(24);
    SETCOLOR(27);
    SETCOLOR(30);
    CopNoOpData(cp, i);
    CopNoOpData(cp, i);
    CopNoOpData(cp, i);
  }
}

static void InitStaticCommandsCopperList(CopListT *cp) {
  short y;

  CopInit(cp);
  for (y = STATIC_Y_START; y < BOOK_Y; y++) {
    CopWait(cp, y, COPWAIT_X);
    CopNoOp(cp);
  }
  CopMove32(cp, cop2lc, bottomCpStart);
  CopMove16(cp, copjmp2, 0);
}

static void InitBottomCopperList(CopListT *cp) {
  cp = NewCopList(80 + STATIC_Y_AREA * 2);
  CopInit(cp);
  bottomCpStart = cp->curr;
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

static void InitCopperList(BallCopListT *ballCp) {
  short i, k;

  CopListT *cp           = NewCopList(INSTRUCTIONS_PER_BALL * 3 + 100);
  CopListT *staticAreaCp = NewCopList(10 + STATIC_Y_AREA * 2);

  // Static area: list of (Y-wait, command) pairs, shown when no ball is overlapping

  InitStaticCommandsCopperList(staticAreaCp);
  ballCp->staticAreaCopperStart = staticAreaCp->entry;

  // Main copper list
  //
  // - Display setup
  // - Sprite setup
  // - Jump back and forth to display balls and static Y area
  // - Jump to book at bottom

  ballCp->cp = cp;
  CopInit(cp);
  CopSetupDisplayWindow(cp, MODE_LORES, X(0), Y0, 320, 280);
  CopMove16(cp, dmacon, DMAF_SETCLR | DMAF_RASTER | DMAF_SPRITE);
  CopSetColor(cp, 0, 0x134);
  CopSetColor(cp, 1, 0x000);
  CopSetColor(cp, 17, 0x49c); // background sprites
  CopSetColor(cp, 18, 0x368);
  CopSetColor(cp, 21, 0x38b);
  CopSetColor(cp, 22, 0x357);
  CopSetColor(cp, 25, 0x6cf); // top-of-book sprites
  CopSetColor(cp, 29, 0x6cf);
  CopSetupBitplaneFetch(cp, MODE_LORES, X(0), ball_large.width);
  CopMove32(cp, bplpt[0], ball_large.planes[0]);
  CopMove16(cp, bpl1mod, -WIDTH/8);
  CopMove16(cp, bpl2mod, -WIDTH/8);
  CopMove16(cp, bplcon0, BPLCON0_COLOR | BPLCON0_BPU(1));
  CopMove16(cp, bplcon2, 0);
  CopMove16(cp, bplcon3, 0);
  CopSetupSprites(cp, NULL);

  // Copper structure per ball.
  //
  // <<CMD>> = instructions from static Y area
  // <<NOP>> = copper no-op or wait + static Y instruction (depending on ball size)
  //
  // [copperJumpTarget] COP2LCH  COP2LCL
  // [waitBefore]       WAIT_YY  <<CMD>>  BPLCON0
  // [bplptr]           BPL1PTH  BPL1PTL  BPL2PTH  BPL2PTL  BPL3PTH  BPL3PTL  BPL4PTH  BPL4PTL  BPL5PTH  BPL5PTL
  // [bplcon1ins]       BPLCON1  BPLCON0
  // [paddingTop]       WAIT_YY  <<CMD>>  [repeat BALL_PADDING_TOP times]
  // [ballCopper]       WAIT_YY  COLOR03  COLOR03  COLOR07    ...    COLOR27  COLOR31  <<CMD>>  <<NOP>>  <<NOP>>
  //                    WAIT_YY  COLOR02  COLOR04  COLOR06    ...    COLOR24  COLOR28  <<CMD>>  <<NOP>>  <<NOP>>
  //                      ...
  //                    WAIT_YY  COLOR03  COLOR03  COLOR07    ...    COLOR27  COLOR31  <<CMD>>  <<NOP>>  <<NOP>>
  //                    WAIT_YY  COLOR02  COLOR04  COLOR06    ...    COLOR24  COLOR28  <<CMD>>  <<NOP>>  <<NOP>>
  // [paddingBottom]    WAIT_YY  <<CMD>>  [repeat BALL_PADDING_BOTTOM times]
  //                    BPLCON0  BPL1MOD  BPL2MOD  COPJMP2
  //

  for (i = 0; i < BALLS; i++) {
    ballCp->inserts[i].copperJumpTarget = CopMove32(cp, cop2lc, bottomCpStart);
    ballCp->inserts[i].waitBefore = CopWait(cp, 0, COPWAIT_X_BALLSTART);
    CopNoOp(cp);
    CopMove16(cp, bplcon0, BPLCON0_COLOR | BPLCON0_BPU(0));
    CopSetupBitplanes(cp, ballCp->inserts[i].bplptr, &ball_large, ball_large.depth);
    ballCp->inserts[i].bplcon1ins = CopMove16(cp, bplcon1, 0);
    CopMove16(cp, bplcon0, BPLCON0_COLOR | BPLCON0_BPU(DEPTH));
    ballCp->inserts[i].paddingTop = cp->curr;
    for (k = 0; k < BALL_PADDING_TOP; k++) {
      CopWait(cp, 0, COPWAIT_X_BALLSTART);
      CopNoOp(cp);
    }
    ballCp->inserts[i].ballCopper = cp->curr;
    InsertTextureCopperWrites(cp);
    ballCp->inserts[i].paddingBottom = cp->curr;
    for (k = 0; k < BALL_PADDING_BOTTOM; k++) {
      CopWait(cp, 0, COPWAIT_X_BALLSTART);
      CopNoOp(cp);
    }
    CopMove16(cp, bplcon0, BPLCON0_COLOR | BPLCON0_BPU(1));
    CopMove16(cp, bpl1mod, -WIDTH/8);
    CopMove16(cp, bpl2mod, -WIDTH/8);
    CopMove16(cp, copjmp2, 0);
  }
}

static inline void SkipBall(BallCopInsertsT *inserts, CopInsT *jumpTo) {
  CopInsSet32(inserts->copperJumpTarget, jumpTo);
  inserts->waitBefore->move.reg = CSREG(copjmp2);
}

static inline void DrawCopperBallTexture(BallT *ball, BallCopInsertsT *inserts, BallCopInsertsT *lastInserts, bool drawTexture) {
  short sin, cos;
  int u, v;
  short zoom = ball->zoom + ((ball->zoomSinAmp * SIN(ball->zoomSinPos)) >> 12);

  sin = (zoom*SIN(ball->angle)) >> (4 + ZOOM_SHIFT);
  cos = (zoom*COS(ball->angle)) >> (4 + ZOOM_SHIFT);
  u = ball->u - sin * (ROTZOOM_W / 2) - cos * (ROTZOOM_H / 2);
  v = ball->v - cos * (ROTZOOM_W / 2) + sin * (ROTZOOM_H / 2);

  if (drawTexture || !lastInserts) {
    PlotTextureAsm((char *) inserts->ballCopper, (char *) ball->texture.pixels, u, v, sin, cos, cos, -sin);
  } else {
    CopyTextureAsm((char *) lastInserts->ballCopper, (char *) inserts->ballCopper);
  }

  ball->angle += ball->angleDelta;
  ball->u += ball->uDelta;
  ball->v += ball->vDelta;
  ball->zoomSinPos += ball->zoomSinStep;
}

// TODO UpdateBallCopper leads to a crash unless wrapped in another function like this, even it's just an rts -- why?

static void CallUpdateCopper(short y, short yInc, CopInsT *staticYSource, CopInsT *textureCopper, CopInsT *paddingTop, CopInsT *paddingBottom) {
  UpdateBallCopper(y, yInc, staticYSource, textureCopper, paddingTop, paddingBottom);
}

static void DrawCopperBall(BallT *ball, BallCopInsertsT *inserts, BallCopInsertsT *lastInserts, bool drawTexture) {
  bool small = ball->height == SMALL_BALL_HEIGHT;
  short y = ball->screenY;
  short staticYPos = y - STATIC_Y_START + STATIC_Y_AREA_PADDING + 1;
  short x = (small ? SMALL_BALL_CENTER : LARGE_BALL_CENTER) - ball->screenX;
  short bplSkip = (x / 8) & 0xfe;
  short shift = 15 - (x & 0xf);
  BitmapT bitmap = small ? ball_small : ball_large;

  if (staticYPos < 0) {
    staticYPos = 0;
  } else if (staticYPos > STATIC_Y_AREA_PADDING + STATIC_Y_AREA) {
    staticYPos = STATIC_Y_AREA_PADDING + STATIC_Y_AREA - 1;
  }

  // Set X
  if (ball->screenY <= Y0) {
    bplSkip += (Y0 - 1 - ball->screenY) * WIDTH / 8;
  }
  CopInsSet32(inserts->bplptr[0], bitmap.planes[0]+bplSkip);
  CopInsSet32(inserts->bplptr[1], bitmap.planes[1]+bplSkip);
  CopInsSet32(inserts->bplptr[2], bitmap.planes[2]+bplSkip);
  CopInsSet32(inserts->bplptr[3], bitmap.planes[3]+bplSkip);
  CopInsSet32(inserts->bplptr[4], bitmap.planes[4]+bplSkip);
  CopInsSet16(inserts->bplcon1ins, (shift << 4) | shift);

  // Update copper waits according to Y

  if (y > Y0) {
    inserts->waitBefore->wait.vp = y;
  } else {
    inserts->waitBefore->wait.vp = 0;
  }
  inserts->waitBefore->wait.hp = COPWAIT_X_BALLSTART | 1;

  // Update non-texture copper commands (waits, static Y area)

  if (staticYPos > 0)
    staticYPos--;

  CallUpdateCopper(y,
                   small ? SMALL_BALL_Y_INC : LARGE_BALL_Y_INC,
                   staticYCommands + staticYPos,
                   inserts->ballCopper,
                   inserts->paddingTop,
                   inserts->paddingBottom);

  DrawCopperBallTexture(ball, inserts, lastInserts, drawTexture);
}

static void Randomize(BallT *ball) {
  ball->height = (random() & 1) ? SMALL_BALL_HEIGHT : LARGE_BALL_HEIGHT;
  ball->zoom = NORM_ZOOM;
  ball->zoomSinPos = random();
  ball->zoomSinAmp = NORM_ZOOM * 6;
  ball->zoomSinStep = random() & 0xf;
  ball->uDelta = random() & 0x1ff;
  ball->vDelta = random() & 0x1ff;
  ball->angle = random();
  ball->angleDelta = random() & 0x3f;
  if (ball->angle & 1) ball->angleDelta = -ball->angleDelta;
  ball->screenX = -64 + (random() & 0x7f);
}

// position: 0 1 2 3 4 5 6 7
// sprite:   4 2 5 0 1 6 3 7 --> 3 4 1 6 0 2 5 7

static short sprite_xy[8][10][2] = {
  { {BOOK_X(3)<<SPRITE_COORD_SHIFT,250<<SPRITE_COORD_SHIFT}, {400,200},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0} },
  { {BOOK_X(4)<<SPRITE_COORD_SHIFT,250<<SPRITE_COORD_SHIFT}, {0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0} },
  { {BOOK_X(1)<<SPRITE_COORD_SHIFT,250<<SPRITE_COORD_SHIFT}, {0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0} },
  { {BOOK_X(6)<<SPRITE_COORD_SHIFT,250<<SPRITE_COORD_SHIFT}, {0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0} },
  { {BOOK_X(0)<<SPRITE_COORD_SHIFT,250<<SPRITE_COORD_SHIFT}, {0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0} },
  { {BOOK_X(2)<<SPRITE_COORD_SHIFT,250<<SPRITE_COORD_SHIFT}, {0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0} },
  { {BOOK_X(5)<<SPRITE_COORD_SHIFT,250<<SPRITE_COORD_SHIFT}, {0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0} },
  { {BOOK_X(7)<<SPRITE_COORD_SHIFT,250<<SPRITE_COORD_SHIFT}, {0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0} }
};

static void SetSpritePointers(void) {
  SetSprites((void *) spr0.planes[0]+4*136, CSREG(sprpt[0]), sprite_xy[0]);
  SetSprites((void *) spr1.planes[0]+4*136, CSREG(sprpt[1]), sprite_xy[1]);
  SetSprites((void *) spr2.planes[0]+4*136, CSREG(sprpt[2]), sprite_xy[2]);
  SetSprites((void *) spr3.planes[0]+4*136, CSREG(sprpt[3]), sprite_xy[3]);
  SetSprites((void *) spr4.planes[0], CSREG(sprpt[4]), sprite_xy[4]);
  SetSprites((void *) spr5.planes[0], CSREG(sprpt[5]), sprite_xy[5]);
  SetSprites((void *) spr6.planes[0], CSREG(sprpt[6]), sprite_xy[6]);
  SetSprites((void *) spr7.planes[0], CSREG(sprpt[7]), sprite_xy[7]);
}

static void MoveSprites(void) {
  short i, step;

  for (i = 0; i < 4; i++) {
    step = 4-i;
    sprite_xy[i][1][1] -= step;
    sprite_xy[i][2][1] -= step;
    sprite_xy[i][3][1] -= step;
    sprite_xy[i][4][1] -= step;
    sprite_xy[i][5][1] -= step;
    sprite_xy[i][6][1] -= step;
    sprite_xy[i][7][1] -= step;
    sprite_xy[i][8][1] -= step;

    if (sprite_xy[i][1][1] < MIN_SPRITE_Y) sprite_xy[i][1][1] = 0;
    if (sprite_xy[i][2][1] < MIN_SPRITE_Y) sprite_xy[i][2][1] = 0;
    if (sprite_xy[i][3][1] < MIN_SPRITE_Y) sprite_xy[i][3][1] = 0;
    if (sprite_xy[i][4][1] < MIN_SPRITE_Y) sprite_xy[i][4][1] = 0;
    if (sprite_xy[i][5][1] < MIN_SPRITE_Y) sprite_xy[i][5][1] = 0;
    if (sprite_xy[i][6][1] < MIN_SPRITE_Y) sprite_xy[i][6][1] = 0;
    if (sprite_xy[i][7][1] < MIN_SPRITE_Y) sprite_xy[i][7][1] = 0;
    if (sprite_xy[i][8][1] < MIN_SPRITE_Y) {
      sprite_xy[i][8][1] = 0;
      if (sprite_xy[i][1][1] < MAX_SPRITE_Y - NEW_SPRITE_PADDING - (sprite_xy[i][1][0] & 0x3f)) {
        sprite_xy[i][8][0] = sprite_xy[i][7][0];
        sprite_xy[i][8][1] = sprite_xy[i][7][1];
        sprite_xy[i][7][0] = sprite_xy[i][6][0];
        sprite_xy[i][7][1] = sprite_xy[i][6][1];
        sprite_xy[i][6][0] = sprite_xy[i][5][0];
        sprite_xy[i][6][1] = sprite_xy[i][5][1];
        sprite_xy[i][5][0] = sprite_xy[i][4][0];
        sprite_xy[i][5][1] = sprite_xy[i][4][1];
        sprite_xy[i][4][0] = sprite_xy[i][3][0];
        sprite_xy[i][4][1] = sprite_xy[i][3][1];
        sprite_xy[i][3][0] = sprite_xy[i][2][0];
        sprite_xy[i][3][1] = sprite_xy[i][2][1];
        sprite_xy[i][2][0] = sprite_xy[i][1][0];
        sprite_xy[i][2][1] = sprite_xy[i][1][1];
        sprite_xy[i][1][0] = (X(160-128) + (random() & 0xff)) << SPRITE_COORD_SHIFT;
        sprite_xy[i][1][1] = MAX_SPRITE_Y;
      }
    }
  }

}

static void Init(void) {
  MouseInit(-100, -200, 100, 256);

  InitStaticYCommands();
  InitBottomCopperList(bottomCp);
  InitCopperList(&ballCopList[0]);
  InitCopperList(&ballCopList[1]);

  Randomize(balls[0]);
  balls[0]->height = LARGE_BALL_HEIGHT;
  balls[0]->screenY = BOOK_Y;
  balls[0]->texture = texture_butterfly;

  Randomize(balls[1]);
  balls[1]->height = SMALL_BALL_HEIGHT;
  balls[1]->screenY = balls[0]->screenY + LARGE_BALL_HEIGHT + 1;
  balls[1]->texture = texture_butterfly;

  Randomize(balls[2]);
  balls[2]->height = SMALL_BALL_HEIGHT;
  balls[2]->screenY = balls[1]->screenY + SMALL_BALL_HEIGHT + 1;
  balls[2]->texture = texture_butterfly;

  custom->dmacon = DMAF_MASTER | DMAF_COPPER | DMAF_SETCLR;
}

static bool MoveBallAndIsStillVisible(BallT *ball) {
  short y = mouseControlled ? ball->screenY : ball->screenY--;
  short lastVisibleY = Y0 - ball->height;
  return y > lastVisibleY;
}

static void Kill(void) {
  MouseKill();
  DeleteCopList(ballCopList[0].cp);
  DeleteCopList(ballCopList[1].cp);
  DeleteCopList(bottomCp);
}

static void HandleEvent(void) {
  EventT ev[1];

  mouseMoved = false;

  if (!PopEvent(ev))
    return;

  if (ev->type == EV_MOUSE) {
    balls[0]->screenX = ev->mouse.x;
    balls[0]->screenY = ev->mouse.y;
    balls[1]->screenY = balls[0]->screenY + LARGE_BALL_HEIGHT + 50;
    balls[2]->screenY = balls[0]->screenY + LARGE_BALL_HEIGHT * 3;
    mouseControlled = true;
    mouseMoved = true;
  }

}

static inline bool IsVisible(BallT *ball) {
  return (ball->screenY + ball->height >= Y0) && (ball->screenY < BOOK_Y - 1);
}

static void DrawBalls(void) {
  BallT *top, *middle, *bottom;

  if (!IsVisible(balls[0])) {
    top    = NULL;
    middle = NULL;
    bottom = NULL;
  } else if (!IsVisible(balls[1])) {
    DrawCopperBall(balls[0], &ballCopList[active].inserts[2], NULL, true);
    top    = NULL;
    middle = NULL;
    bottom = balls[0];
  } else if (!IsVisible(balls[2])) {
    DrawCopperBall(balls[0], &ballCopList[active].inserts[1], NULL, true);
    DrawCopperBall(balls[1], &ballCopList[active].inserts[2], NULL, true);
    top    = NULL;
    middle = balls[0];
    bottom = balls[1];
    lastInsertsTop    = NULL;
    lastInsertsBottom = NULL;
  } else {
    // Skip tx mapping every other frame when 3 balls are visible, alternating
    DrawCopperBall(balls[0], &ballCopList[active].inserts[0], lastInsertsTop, active);
    DrawCopperBall(balls[1], &ballCopList[active].inserts[1], NULL, true);
    DrawCopperBall(balls[2], &ballCopList[active].inserts[2], lastInsertsBottom, !active);
    top    = balls[0];
    middle = balls[1];
    bottom = balls[2];
    lastInsertsTop    = &ballCopList[active].inserts[0];
    lastInsertsBottom = &ballCopList[active].inserts[2];
  }

  // Bottom ball

  if (bottom == NULL) {
    SkipBall(&ballCopList[active].inserts[0], ballCopList[active].staticAreaCopperStart);
  } else {
    CopInsT *bottomBallCopperStart;
    short bottomBallYEnd;
    bottomBallYEnd = bottom->screenY + bottom->height;
    if (bottom->screenY > STATIC_Y_START) {
      bottomBallCopperStart = ballCopList[active].staticAreaCopperStart;
      bottomBallYEnd = bottom->screenY + bottom->height;
      CopInsSet32(ballCopList[active].inserts[2].copperJumpTarget, bottomCpStart);
    } else {
      short staticYCovered = bottomBallYEnd - STATIC_Y_START;
      bottomBallCopperStart = ballCopList[active].inserts[2].copperJumpTarget;
      if (staticYCovered > 0) {
        if (staticYCovered > STATIC_Y_AREA) staticYCovered = STATIC_Y_AREA;
        CopInsSet32(ballCopList[active].inserts[2].copperJumpTarget, ballCopList[active].staticAreaCopperStart + staticYCovered * 2);
      } else {
        CopInsSet32(ballCopList[active].inserts[2].copperJumpTarget, ballCopList[active].staticAreaCopperStart);
      }
    }

    // Middle ball

    if (middle == NULL) {
      // No middle ball: jump to bottom ball directly
      SkipBall(&ballCopList[active].inserts[0], bottomBallCopperStart);
    } else {
      // Middle ball start, exit
      short middleBallYEnd = middle->screenY + middle->height;
      short staticYCovered = middleBallYEnd - STATIC_Y_START;
      if (staticYCovered > 0) {
        if (staticYCovered > STATIC_Y_AREA) staticYCovered = STATIC_Y_AREA;
        CopInsSet32(ballCopList[active].inserts[1].copperJumpTarget, ballCopList[active].staticAreaCopperStart + staticYCovered * 2);
      } else {
        CopInsSet32(ballCopList[active].inserts[1].copperJumpTarget, bottomBallCopperStart);
      }

      // Top ball

      if (top == NULL) {
        SkipBall(&ballCopList[active].inserts[0], ballCopList[active].inserts[1].copperJumpTarget);
      } else {
        CopInsSet32(ballCopList[active].inserts[0].copperJumpTarget, ballCopList[active].inserts[1].copperJumpTarget);
      }
    }
  }

  // Static Y area, possibly containing a COP2JMP before the end

  WriteStaticYArea(ballCopList[active].staticAreaCopperStart,
                   staticYCommands + STATIC_Y_AREA_PADDING,
                   bottom == NULL ? -1 : bottom->screenY,
                   ballCopList[active].inserts[2].copperJumpTarget,
                   bottomCpStart);

}

static void Render(void) {
  SetSpritePointers();
  HandleEvent();
  MoveBallAndIsStillVisible(balls[1]);
  MoveBallAndIsStillVisible(balls[2]);
  if (!MoveBallAndIsStillVisible(balls[0]) && !mouseControlled) {
    short lastY = balls[2]->screenY + balls[2]->height;
    BallT *tmp = balls[0];
    balls[0] = balls[1];
    balls[1] = balls[2];
    balls[2] = tmp;
    balls[2]->screenY = lastY + 1 + (random() &0x3f);
    if (balls[2]->screenY < BOOK_Y)
      balls[2]->screenY = BOOK_Y + (random() & 0x3f);
    Randomize(balls[2]);
    lastInsertsTop    = NULL;
    lastInsertsBottom = NULL;
  }
  DrawBalls();
  MoveSprites();
  TaskWaitVBlank();
  active ^= 1;
  CopListRun(ballCopList[active].cp);
}

EffectT Effect = { NULL, NULL, Init, Kill, Render };
