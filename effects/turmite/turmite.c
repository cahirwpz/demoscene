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
  u_char rules[2][2][2];
} TurmiteT;

/* board color is assumed to be binary */
#define RULE(ncolor, ndir, nstate) \
  { (nstate) * 2 + (ncolor), (ndir) }
#define GETRULE(rules, state, color) \
  ((u_char *)(rules) + ((state) | (color)) * 2)
#define POS(x, y) ((y) * WIDTH + (x))
#define RCOLOR(r) ((r)[0] & 1)
#define RSTATE(r) ((r)[0] & -2)
#define RDIR(r) ((r)[1])

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

/* static inline */
void TurmiteMove(TurmiteT *t, u_char *board, u_char *bpl) {
  u_int pos = t->pos;
  u_char col = board[pos];
  u_char *rule = GETRULE(t->rules, t->state, col);
  u_short dir;

  board[pos] = RCOLOR(rule);
  t->state = RSTATE(rule);

  {
    u_int offset = pos >> 3;
    u_char bit = ~pos;
    if (t->state) {
      bset(bpl + offset, bit);
    } else {
      bclr(bpl + offset, bit);
    }
  }

  dir = t->dir;
  dir += RDIR(rule);
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
  u_char *bpl = screen->planes[0]; 
  short i;

  for (i = 0; i < NSTEPS; i++) {
    TurmiteMove(&ChaoticGrowth, board, bpl);
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
