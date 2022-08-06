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

typedef struct Rule {
  short ncolor;
  short ndir;
  short nstate;
} RuleT;

#define RULESZ sizeof(RuleT)

typedef struct Turmite {
  u_short pos;
  short state;
  short dir;
  RuleT rules[2][2];
} TurmiteT;

/* board color is assumed to be binary */
#define RULE(nc, nd, ns) \
  (RuleT){ .ncolor = (nc) * RULESZ, .ndir = (nd), .nstate = (ns) * RULESZ * 2 }
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

static const u_short PosChange[4] = { +WIDTH, -1, -WIDTH, +1 };

static inline void FlipPixel(u_char *bpl, u_int pos) {
  short offset = pos >> 3;
  u_char bit = ~pos;
  bchg(&bpl[offset], bit);
}

static inline RuleT *GetRule(TurmiteT *t, short col) {
  void *rule = t->rules;
  short offset = col + t->state;
  return rule + offset;
}

#if 0
static inline
#endif
void TurmiteMove(TurmiteT *t, u_char *board, u_char *bpl) {
  int pos = t->pos;
  short col = board[pos];
  RuleT *rule = GetRule(t, col);

  t->state = rule->nstate;

  {
    short newcol = rule->ncolor;
    board[pos] = newcol;

    if (newcol != col)
      FlipPixel(bpl, pos);
  }

  {
    short dir = (t->dir + rule->ndir) & 3;

    t->dir = dir;
    t->pos += PosChange[dir];
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
