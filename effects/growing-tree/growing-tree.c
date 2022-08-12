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
#define DEPTH 2

static CopListT *cp;
static CopInsT *bplptr[DEPTH];
static BitmapT *screen;

typedef struct Branch {
  short pos_x, pos_y; // Q12.4
  union { // Q4.12
    short word;
    char byte;
  } _vel_x;
  union { // Q4.12
    short word;
    char byte;
  } _vel_y;
  short diameter;     // Q12.4
} BranchT;

#define vel_x _vel_x.word
#define vel_y _vel_y.word
#define vel_x_b _vel_x.byte
#define vel_y_b _vel_y.byte

#define MAXBRANCHES 256

static BranchT branches[MAXBRANCHES];
static BranchT *lastBranch = branches;

static u_short fastrand(void) {
  static int ma = 0x3E50B28C;
  static int mb = 0xD461A7F9;

  int a = ma;
  int b = mb;

  b = swap16(b);

  ma += b;
  mb += a;
  
  return a;
}

#define random fastrand

static void Init(void) {
  screen = NewBitmap(WIDTH, HEIGHT, DEPTH);

  SetupPlayfield(MODE_LORES, DEPTH, X(0), Y(0), WIDTH, HEIGHT);
  SetColor(0, 0xfff);
  SetColor(1, 0x000);
  SetColor(2, 0xf00);
  SetColor(3, 0xf00);

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

static inline BranchT *NewBranch(void) {
  if (lastBranch == &branches[MAXBRANCHES]) {
    return NULL;
  } else {
    return lastBranch++;
  }
}

static void MakeBranch(short x, short y) {
  BranchT *b = NewBranch();
  if (!b)
    return;
  b->pos_x = fx4i(x);
  b->pos_y = fx4i(y);
  b->vel_x = fx12i(0);
  b->vel_y = fx12i(-7);
  b->diameter = fx4i(20);
}

static inline void KillBranch(BranchT *b, BranchT **lastp) {
  BranchT *last = --(*lastp);
  if (b != last)
    *b = *last;
}

static void SplitBranch(BranchT *parent, BranchT **lastp) {
  short newDiameter = normfx(mul16(parent->diameter, fx12f(0.6)));

  if (newDiameter < fx4f(0.2)) {
    KillBranch(parent, lastp);
  } else {
    BranchT *b = NewBranch();
    if (!b)
      return;
    b->pos_x = parent->pos_x;
    b->pos_y = parent->pos_y;
    b->vel_x = parent->vel_x;
    b->vel_y = parent->vel_y;
    b->diameter = newDiameter;
    parent->diameter = newDiameter;
  }
}

static inline int shift14(short a) {
  int b;
  asm("swap %0\n\t"
      "clrw %0\n\t"
      "asrl #2,%0"
      : "=d" (b) : "0" (a));
  return b;
}

/*
 * float scale = random(1.0, 2.0);
 * float bump_x = random(-1.0, 1.0) * scale;
 * float bump_y = random(-1.0, 1.0) * scale;
 * float mag = abs(velocity.x) + abs(velocity.y);
 * velocity.mult(scale * 4.0 / mag);
 * velocity.add(bump_x, bump_y);
 * position.add(velocity);
 *
 * assert(velocity.x > -8.0 && velocity.x < 8.0);
 * assert(velocity.y > -8.0 && velocity.y < 8.0);
 * assert(mag > 2.0 && mag < 12.0);
 */
void GrowingTree(BranchT *branches, BranchT **lastp) {
  BranchT *b;

  for (b = *lastp - 1; b >= branches; b--) {
    short prev_x = b->pos_x;
    short prev_y = b->pos_y;
    short curr_x, curr_y;

    {
      short scale = (random() & 0xfff) + 0x1000; // Q4.12
      short bump_x = (random() & 0x3fff) - 0x2000; // Q4.12
      short bump_y = (random() & 0x3fff) - 0x2000; // Q4.12
      short vx = b->vel_x;
      short vy = b->vel_y;
      short vel_scale = shift14(scale) / (abs(vx) + abs(vy));

      b->vel_x = bump_x + normfx(vel_scale * vx);
      b->vel_y = bump_y + normfx(vel_scale * vy);

      curr_x = prev_x + b->vel_x_b;
      curr_y = prev_y + b->vel_y_b;
    }

    if (curr_x < 0 || curr_x >= fx4i(WIDTH) ||
        curr_y < 0 || curr_y >= fx4i(HEIGHT))
    {
      KillBranch(b, lastp);
    } else {
      BlitterLine(prev_x >> 4, prev_y >> 4, curr_x >> 4, curr_y >> 4);

      b->pos_x = curr_x;
      b->pos_y = curr_y;

      if ((u_short)random() < (u_short)(65536 * 0.2f)) {
        SplitBranch(b, lastp);
      }
    }
  }
}

PROFILE(GrowTree);

static void Render(void) {
  BlitterLineSetup(screen, 0, LINE_OR|LINE_SOLID);

  if (lastBranch == branches) {
    BitmapClear(screen);
    MakeBranch(WIDTH / 2, HEIGHT - 1);
  }

  ProfilerStart(GrowTree);
  GrowingTree(branches, &lastBranch);
  ProfilerStop(GrowTree);

  TaskWaitVBlank();
}

EFFECT(growing_tree, NULL, NULL, Init, Kill, Render);
