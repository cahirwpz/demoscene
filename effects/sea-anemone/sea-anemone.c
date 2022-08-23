#include <effect.h>
#include <blitter.h>
#include <copper.h>
#include <fx.h>
#include <gfx.h>
#include <line.h>
#include <stdlib.h>
#include <system/memory.h>

#define WIDTH 320
#define HEIGHT 256
#define DEPTH 1

static CopListT *cp;
static CopInsT *bplptr[DEPTH];
static BitmapT *screen;

#include "data/anemone-pal.c"

static inline int fastrand(void) {
  static int m[2] = { 0x3E50B28C, 0xD461A7F9 };

  int a, b;

  // https://www.atari-forum.com/viewtopic.php?p=188000#p188000
  asm volatile("move.l (%2)+,%0\n"
               "move.l (%2),%1\n"
               "swap   %1\n"
               "add.l  %0,(%2)\n"
               "add.l  %1,-(%2)\n"
               : "=d" (a), "=d" (b)
               : "a" (m));
  
  return a;
}

#define random fastrand

#define DIAMETER 48
#define NARMS 31 /* must be power of two minus one */

typedef struct Arm {
  short diameter;
  union { // Q4.12
    short word;
    char byte;
  } _vel_x;
  union { // Q4.12
    short word;
    char byte;
  } _vel_y;
  short pos_x, pos_y; // Q12.4
  short pad[3];
} ArmT;

#define vel_x _vel_x.word
#define vel_y _vel_y.word
#define vel_x_b _vel_x.byte
#define vel_y_b _vel_y.byte

typedef struct ArmQueue {
  short head; // points to first unused entry
  short tail; // points to the last element
  ArmT elems[NARMS + 1]; // One guard element
} ArmQueueT;

static inline ArmT *ArmGet(ArmQueueT *arms, short i) {
  return &arms->elems[i & NARMS];
}

static inline ArmT *ArmFirst(ArmQueueT *arms) {
  return ArmGet(arms, arms->head - 1);
}

static inline ArmT *ArmLast(ArmQueueT *arms) {
  return ArmGet(arms, arms->tail);
}

static inline ArmT *ArmPrev(ArmQueueT *arms, ArmT *arm) {
  arm--;
  return arm < arms->elems ? &arms->elems[NARMS] : arm;
}

static inline bool ArmsNonEmpty(ArmQueueT *arms) {
  return arms->head != arms->tail;
}

static inline bool ArmsFull(ArmQueueT *arms) {
  return ((arms->head + 1) & NARMS) == arms->tail;
}

static void MakeArm(ArmT *arm) {
  arm->pos_x = fx4i(WIDTH / 2);
  arm->pos_y = fx4i(HEIGHT / 2);
  arm->vel_x = 0;
  arm->vel_y = 0;
  arm->diameter = mod16(random() & 0x7fff, DIAMETER / 4) + DIAMETER * 3 / 4;
}

static void ArmsAdd(ArmQueueT *arms, ArmT *arm) {
  short prev, curr;

  if (ArmsFull(arms))
    return;

  prev = (arms->head - 1) & NARMS;
  curr = arms->head;

  while (curr != arms->tail && arm->diameter <= arms->elems[prev].diameter) {
    arms->elems[curr] = arms->elems[prev];      
    curr = prev;
    prev = (prev - 1) & NARMS;
  }

  arms->elems[curr] = *arm;
  arms->head = (arms->head + 1) & NARMS;
}

static void ArmsPop(ArmQueueT *arms) {
  arms->tail = (arms->tail + 1) & NARMS;
}

static void ArmMove(ArmT *arm) {
  short angle = random();
  short vx = arm->vel_x;
  short vy = arm->vel_y;
  int magSq;
  int diameter;

  vx += COS(angle) >> 1;
  vy += SIN(angle) >> 1;

  magSq = vx * vx + vy * vy; // Q8.24
  diameter = arm->diameter << 24;

  if (magSq > diameter) {
    // short scale = div16(diameter, normfx(magSq));
    // `magSq` does get truncated if converted to 16-bit,
    // so it's not eligible for 16-bit division.
    short scale = diameter / (magSq >> 12);
    vx = normfx(vx * scale);
    vy = normfx(vy * scale);
  }

  arm->vel_x = vx;
  arm->vel_y = vy;

  arm->pos_x += arm->vel_x_b;
  arm->pos_y += arm->vel_y_b;

  arm->diameter--;
}

static void DrawPoint(short x, short y) {
  u_int bit = y * WIDTH + x;
  bset((u_char *)screen->planes[0] + bit / 8, ~bit);
}

static void Init(void) {
  screen = NewBitmap(WIDTH, HEIGHT, DEPTH);

  SetupPlayfield(MODE_LORES, DEPTH, X(0), Y(0), WIDTH, HEIGHT);
  LoadPalette(&anemone_pal, 0);
  SetColor(1, anemone_pal.colors[15]);

  cp = NewCopList(50);
  CopInit(cp);
  CopSetupBitplanes(cp, bplptr, screen, DEPTH);
  CopEnd(cp);
  CopListActivate(cp);

  EnableDMA(DMAF_RASTER | DMAF_BLITTER);
}

static void Kill(void) {
  DisableDMA(DMAF_COPPER | DMAF_BLITTER | DMAF_RASTER);

  DeleteBitmap(screen);
  DeleteCopList(cp);
}

#define screen_bytesPerRow (WIDTH / 8)

void SeaAnemone(ArmQueueT *arms) {
  static ArmT arm;

  MakeArm(&arm);
  ArmsAdd(arms, &arm);

  if (ArmsNonEmpty(arms)) {
    ArmT *curr = ArmFirst(arms);
    ArmT *last = ArmLast(arms);

    while (true) {
      ArmMove(curr);
      DrawPoint(curr->pos_x >> 4, curr->pos_y >> 4);
      if (curr == last)
        break;
      curr = ArmPrev(arms, curr);
    }

    // Log("head: %d, tail: %d\n", arms.head, arms.tail);

    while (ArmsNonEmpty(arms) && ArmLast(arms)->diameter < 1) {
      ArmsPop(arms);
    }
  }
}

PROFILE(SeaAnemone);

static void Render(void) {
  static ArmQueueT arms;

  ProfilerStart(SeaAnemone);
  SeaAnemone(&arms);
  ProfilerStop(SeaAnemone);

  TaskWaitVBlank();
}

EFFECT(SeaAnemone, NULL, NULL, Init, Kill, Render);
