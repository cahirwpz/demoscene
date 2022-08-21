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

#include "data/fruit.c"

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

#define MAXBRANCHES 75

static BranchT branches[MAXBRANCHES];
static BranchT *lastBranch = branches;

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

#define screen_bytesPerRow (WIDTH / 8)

static void CopyFruit(u_short x, u_short y) {
  u_short dstmod = screen_bytesPerRow - fruit_bytesPerRow;
  u_short bltshift = rorw(x & 15, 4);
  u_short bltsize = (fruit_height << 6) | (fruit_bytesPerRow >> 1);
  void *srcbpt = fruit.planes[0];
  void *dstbpt = screen->planes[1];
  u_short bltcon0;

  if (bltshift)
    bltsize++, dstmod -= 2;

  dstbpt += (x & ~15) >> 3;
  dstbpt += y * screen_bytesPerRow;
  bltcon0 = (SRCA | SRCB | DEST | A_OR_B) | bltshift;

  WaitBlitter();

  if (bltshift) {
    custom->bltalwm = 0;
    custom->bltamod = -2;
  } else {
    custom->bltalwm = -1;
    custom->bltamod = 0;
  }

  custom->bltbmod = dstmod;
  custom->bltdmod = dstmod;
  custom->bltcon0 = bltcon0;
  custom->bltcon1 = 0;
  custom->bltafwm = -1;
  custom->bltapt = srcbpt;
  custom->bltbpt = dstbpt;
  custom->bltdpt = dstbpt;
  custom->bltsize = bltsize;
}

static void DrawBranch(short x1, short y1, short x2, short y2) {
  u_char *data = screen->planes[0];
  short dx, dy, derr;

  u_short bltcon0 = BC0F_LINE_OR;
  u_short bltcon1 = LINEMODE;

  /* Always draw the line downwards. */
  if (y1 > y2) {
    swapr(x1, x2);
    swapr(y1, y2);
  }

  bltcon0 |= rorw(x1 & 15, 4);

  /* Word containing the first pixel of the line. */
  data += screen_bytesPerRow * y1;
  data += (x1 >> 3) & ~1;

  dx = x2 - x1;
  dy = y2 - y1;

  if (dx < 0) {
    dx = -dx;
    if (dx >= dy) {
      bltcon1 |= AUL | SUD;
    } else {
      bltcon1 |= SUL;
      swapr(dx, dy);
    }
  } else {
    if (dx >= dy) {
      bltcon1 |= SUD;
    } else {
      swapr(dx, dy);
    }
  }

  derr = dy + dy - dx;
  if (derr < 0)
    bltcon1 |= SIGNFLAG;

  {
    u_short bltamod = derr - dx;
    u_short bltbmod = dy + dy;
    u_short bltsize = (dx << 6) + 66;

    WaitBlitter();

    custom->bltafwm = -1;
    custom->bltalwm = -1;
    custom->bltadat = 0x8000;
    custom->bltbdat = -1;
    custom->bltcmod = screen_bytesPerRow;
    custom->bltdmod = screen_bytesPerRow;

    custom->bltcon0 = bltcon0;
    custom->bltcon1 = bltcon1;
    custom->bltamod = bltamod;
    custom->bltbmod = bltbmod;
    custom->bltapt = (void *)(int)derr;
    custom->bltcpt = data;
    custom->bltdpt = data;
    custom->bltsize = bltsize;
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

static bool SplitBranch(BranchT *parent, BranchT **lastp) {
  short newDiameter = normfx(mul16(parent->diameter, fx12f(0.58)));

  if (newDiameter >= fx4f(0.2)) {
    BranchT *b = NewBranch();
    if (!b)
      return true;
    b->pos_x = parent->pos_x;
    b->pos_y = parent->pos_y;
    b->vel_x = parent->vel_x;
    b->vel_y = parent->vel_y;
    b->diameter = newDiameter;
    parent->diameter = newDiameter;
    return true;
  }

  KillBranch(parent, lastp);
  return false;
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
 * float scale = random(1.0, 1.5);
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
      short scale = (random() & 0x7ff) + 0x1000; // Q4.12
      short bump_x = (random() & 0x1fff) - 0x1000; // Q4.12
      short bump_y = (random() & 0x1fff) - 0x1000; // Q4.12
      short vx = b->vel_x;
      short vy = b->vel_y;
      short vel_scale = div16(shift14(scale), (abs(vx) + abs(vy)));

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
      b->pos_x = curr_x;
      b->pos_y = curr_y;

      prev_x >>= 4;
      prev_y >>= 4;
      curr_x >>= 4;
      curr_y >>= 4;
      
      DrawBranch(prev_x, prev_y, curr_x, curr_y);

      if ((u_short)random() < (u_short)(65536 * 0.2f)) {
        if (!SplitBranch(b, lastp) && b->diameter < fx4f(0.3)) {
          curr_x -= fruit_width / 8;
          curr_y -= fruit_height / 8;

          if ((curr_x >= 0) && (curr_x < WIDTH - fruit_width) &&
              (curr_y >= 0) && (curr_y < HEIGHT - fruit_height))
            CopyFruit(curr_x, curr_y);
        }
      }
    }
  }
}

PROFILE(GrowTree);

static void Render(void) {
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
