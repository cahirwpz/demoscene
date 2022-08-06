#include "effect.h"

#include "custom.h"
#include "copper.h"
#include "blitter.h"
#include "bitmap.h"
#include "sprite.h"
#include "fx.h"

#include "data/turmite-pal.c"

#define WIDTH 256
#define HEIGHT 256
#define DEPTH 1
#define NSTEPS 256

static CopListT *cp;
static BitmapT *screen;
static u_char board[WIDTH * HEIGHT];

static void Init(void) {
  screen = NewBitmap(WIDTH, HEIGHT, DEPTH);

  SetupDisplayWindow(MODE_LORES, X(32), Y(0), WIDTH, HEIGHT);
  SetupBitplaneFetch(MODE_LORES, X(32), WIDTH);
  SetupMode(MODE_LORES, DEPTH);
  LoadPalette(&turmite_pal, 0);

  custom->color[1] = turmite_pal.colors[8];

  cp = NewCopList(100);
  CopInit(cp);
  CopSetupBitplanes(cp, NULL, screen, DEPTH);
  CopEnd(cp);
  CopListActivate(cp);

  EnableDMA(DMAF_RASTER);
}

static void Kill(void) {
  DisableDMA(DMAF_RASTER);
  DeleteCopList(cp);
}

#define SOUTH 0
#define WEST 1
#define NORTH 2
#define EAST 3

typedef struct Turmite {
  u_short pos;
  u_short dir, state;
  u_char rules[2][2][4];
} TurmiteT;

/* board color is assumed to be binary */
#define RULE(col, dir, state) { (col), (dir), (state) }
#define POS(x, y) ((y) * WIDTH + (x))

TurmiteT SpiralGrowth = {
  .pos = POS(128, 128),
  .dir = 0,
  .state = 0,
  .rules = {
    {
      RULE(1,  0, 1),
      RULE(1, -1, 0),
    }, {
      RULE(1, +1, 1),
      RULE(0,  0, 0),
    }
  }
};

TurmiteT ChaoticGrowth = {
  .pos = POS(128, 128),
  .dir = 0,
  .state = 0,
  .rules = {
    {
      RULE(1, +1, 1),
      RULE(1, -1, 1),
    }, {
      RULE(1, +1, 1),
      RULE(0, +1, 0),
    }
  }
};

void TurmiteMove(TurmiteT *t) {
  u_int pos = t->pos;
  u_char col = board[pos];
  u_char *change = t->rules[t->state][col];
  u_short dir;

  board[pos] = change[0];
  t->state = change[2];

  {
    u_char *bpl = screen->planes[0]; 
    u_int offset = pos >> 3;
    u_char bit = ~pos;
    if (t->state) {
      bset(bpl + offset, bit);
    } else {
      bclr(bpl + offset, bit);
    }
  }

  dir = t->dir;
  dir += change[1];
  dir &= 3;

  t->dir = dir;

  if (dir == SOUTH) {
    t->pos += WIDTH;
  } else if (dir == WEST) {
    t->pos--;
  } else if (dir == NORTH) {
    t->pos -= WIDTH;
  } else if (dir == EAST) {
    t->pos++;
  }
}

/* static */ void SimulateTurmite(void) {
  short i;

  for (i = 0; i < NSTEPS; i++) {
    TurmiteMove(&ChaoticGrowth);
  }
}

PROFILE(SimulateTurmite);

static void Render(void) {
  ProfilerStart(SimulateTurmite);
  SimulateTurmite();
  ProfilerStop(SimulateTurmite);

  WaitVBlank();
}

EFFECT(turmite, NULL, NULL, Init, Kill, Render);
