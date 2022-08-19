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
#define DEPTH 4
#define NSTEPS 256

#define GENERATION 1

/* don't decrease it since color's value is stored on lower 3 bits of tile */
#define STEP 8

static CopListT *cp;
static BitmapT *screen;

/* higher 5 bits store value, lower 3 bits store color information */
static u_char board[WIDTH * HEIGHT];

static void Init(void) {
  screen = NewBitmap(WIDTH, HEIGHT, DEPTH);

  SetupDisplayWindow(MODE_LORES, X(32), Y(0), WIDTH, HEIGHT);
  SetupBitplaneFetch(MODE_LORES, X(32), WIDTH);
  SetupMode(MODE_LORES, DEPTH);
  LoadPalette(&turmite_pal, 0);

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
  RuleT rules[3][2];
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

TurmiteT SnowFlake = {
  .pos = POS(128, 128),
  .dir = 0,
  .state = 0,
  .rules = {
    {
      RULE(1, -1, 1),
      RULE(1, +1, 0),
    }, {
      RULE(1, +2, 1),
      RULE(1, +2, 2),
    }, {
      RULE(0, +2, 0),
      RULE(0, +2, 0),
    }
  }
};

TurmiteT Irregular = {
  .pos = POS(128, 128),
  .dir = 0,
  .state = 0,
  .rules = {
    {
      RULE(1, +1, 0),
      RULE(1, +1, 1),
    }, {
      RULE(0, 0, 0),
      RULE(0, 0, 1),
    }
  }
};

static const u_short PosChange[4] = {
  [SOUTH] = +WIDTH,
  [WEST] = -1,
  [NORTH] = -WIDTH,
  [EAST] = +1,
};

#define BPLSIZE (WIDTH * HEIGHT / 8)

static inline void BitValue(u_char *bpl, short offset, u_char bit, u_char val) {
  if (val)
    bset(bpl + offset, bit);
  else
    bclr(bpl + offset, bit);
}

static inline void PixelValue(u_char *bpl, u_char bit, u_char val) {
  BitValue(bpl, BPLSIZE * 0, bit, val & 1);
  BitValue(bpl, BPLSIZE * 1, bit, val & 2);
  BitValue(bpl, BPLSIZE * 2, bit, val & 4);
  BitValue(bpl, BPLSIZE * 3, bit, val & 8);
}

static inline void SetPixel(u_char *bpl, u_short pos, u_char val) {
  int offset = pos >> 3;
  u_char bit = ~pos;

  bpl += offset;
  
  switch (val >> 4) {
    case 0: PixelValue(bpl, bit, 0); break;
    case 1: PixelValue(bpl, bit, 1); break;
    case 2: PixelValue(bpl, bit, 2); break;
    case 3: PixelValue(bpl, bit, 3); break;
    case 4: PixelValue(bpl, bit, 4); break;
    case 5: PixelValue(bpl, bit, 5); break;
    case 6: PixelValue(bpl, bit, 6); break;
    case 7: PixelValue(bpl, bit, 7); break;
    case 8: PixelValue(bpl, bit, 8); break;
    case 9: PixelValue(bpl, bit, 9); break;
    case 10: PixelValue(bpl, bit, 10); break;
    case 11: PixelValue(bpl, bit, 11); break;
    case 12: PixelValue(bpl, bit, 12); break;
    case 13: PixelValue(bpl, bit, 13); break;
    case 14: PixelValue(bpl, bit, 14); break;
    case 15: PixelValue(bpl, bit, 15); break;
  }
}

static inline RuleT *GetRule(TurmiteT *t, short col) {
  void *rule = t->rules;
  short offset = col + t->state;
  return rule + offset;
}

#if GENERATION
static u_char generation = 0;
#endif

static TurmiteT *TheTurmite =
#if GENERATION
  &SnowFlake;
#else
  &Irregular;
#endif

#if 1
static inline
#endif
void TurmiteMove(TurmiteT *t, u_char *board, u_char *bpl) {
  int pos = t->pos;
  short col = board[pos];
  RuleT *rule = GetRule(t, col & (STEP - 1));

  t->state = rule->nstate;

  {
    short newcol = rule->ncolor;
    u_char val;

#if GENERATION
    val = generation;
#else
    val = col & -STEP;
    val += STEP;
    if (val == 0)
       val -= STEP;
    newcol |= val;
#endif

    board[pos] = newcol;

    SetPixel(bpl, pos, val);
  }

  {
    short dir = (t->dir + rule->ndir) & 3;

    t->dir = dir;
    t->pos += PosChange[dir];
  }
}

void SimulateTurmite(void) {
  u_char *bpl = screen->planes[0]; 
  short i;

  for (i = 0; i < NSTEPS; i++) {
    TurmiteMove(TheTurmite, board, bpl);
  }

#if GENERATION
  generation += STEP;
#endif
}

PROFILE(SimulateTurmite);

static void Render(void) {
  ProfilerStart(SimulateTurmite);
  SimulateTurmite();
  ProfilerStop(SimulateTurmite);

  WaitVBlank();
}

EFFECT(Turmite, NULL, NULL, Init, Kill, Render);
