#include "effect.h"

#include "custom.h"
#include "copper.h"
#include "blitter.h"
#include "bitmap.h"
#include "sprite.h"
#include "fx.h"

#include "data/bar.c"
#include "data/colors.c"
#include "data/stripes.c"

#define bar_bplmod ((bar_width - WIDTH) / 8 - 2)

#define WIDTH (320 - 32)
#define HEIGHT 256
#define DEPTH 4
#define NSPRITES 8

#define BELOW 0
#define ABOVE BPLCON2_PF2P2

#define O0 0
#define O1 56
#define O2 112
#define O3 172
#define O4 224

typedef struct State {
  /* at the beginning: 4 bitplane pointers and bplcon1 */
  CopInsT *bar;
  /* for each bar moves to bplcon1, bpl1mod and bpl2mod */
  CopInsT *bar_change[4];
  /* for each line five horizontal positions */
  CopInsT *stripes[HEIGHT];
} StateT;

static int active = 1;
static CopListT *cp[2];
static char sintab8[256];
static StateT state[2];

#define STRIPES 5
#define BARS 4

static char StripePhase[STRIPES] = { 4, 2, 3, 8, 12 };
static char StripePhaseIncr[STRIPES] = { 7, -10, 12, -6, 14 };

static inline void CopSpriteSetHP(CopListT *cp, short n) {
  CopMove16(cp, spr[n * 2 + 0].pos, 0);
  CopMove16(cp, spr[n * 2 + 1].pos, 0);
}

static void MakeCopperList(CopListT *cp, StateT *state) {
  short b, y;

  CopInit(cp);

  /* Setup initial bitplane pointers. */
  state->bar = cp->curr;
  CopMove32(cp, bplpt[0], NULL);
  CopMove32(cp, bplpt[1], NULL);
  CopMove32(cp, bplpt[2], NULL);
  CopMove32(cp, bplpt[3], NULL);
  CopMove16(cp, bplcon1, 0);

  /* Move back bitplane pointers to repeat the line. */
  CopMove16(cp, bpl1mod, -WIDTH / 8 - 2);
  CopMove16(cp, bpl2mod, -WIDTH / 8 - 2);

  /* Setup sprite pointers */
  CopMove32(cp, sprpt[0], &stripes0_sprdat); /* up */
  CopMove32(cp, sprpt[1], &stripes1_sprdat);
  CopMove32(cp, sprpt[2], &stripes2_sprdat); /* down */
  CopMove32(cp, sprpt[3], &stripes3_sprdat);
  CopMove32(cp, sprpt[4], &stripes0_sprdat); /* up */
  CopMove32(cp, sprpt[5], &stripes1_sprdat);
  CopMove32(cp, sprpt[6], &stripes2_sprdat); /* down */
  CopMove32(cp, sprpt[7], &stripes3_sprdat);

  for (y = 0, b = 0; y < HEIGHT; y++) {
    short vp = Y(y);
    short my = y & 63;

    CopWaitSafe(cp, vp, 0);

    /* With current solution bitplane setup takes at most 3 copper move
     * instructions (bpl1mod, bpl2mod, bplcon1) per raster line. */
    if (my == 16) {
      /* Advance bitplane pointers to display consecutive lines. */
      CopMove16(cp, bpl1mod, bar_bplmod);
      CopMove16(cp, bpl2mod, bar_bplmod);
    } else if (my == 48) {
      state->bar_change[b++] = cp->curr;
      /* Move back bitplane pointers to the beginning of bitmap. Take into
       * account new bitmap offset and shifter configuration for next bar. */
      CopMove16(cp, bplcon1, 0);
      CopMove16(cp, bpl1mod, 0);
      CopMove16(cp, bpl2mod, 0);
    } else if (my == 49) {
      /* Move back bitplane pointers to repeat the line. */
      CopMove16(cp, bpl1mod, -WIDTH / 8 - 2);
      CopMove16(cp, bpl2mod, -WIDTH / 8 - 2);
    }

    {
      short p0, p1;

      if (y & 64) {
        p0 = BELOW, p1 = ABOVE;
      } else {
        p0 = ABOVE, p1 = BELOW;
      }

      CopWait(cp, vp, HP(O0) + 2);
      state->stripes[y] = cp->curr;
      CopSpriteSetHP(cp, 0);
      CopMove16(cp, bplcon2, p0);
      CopWait(cp, vp, HP(O1) + 2);
      CopSpriteSetHP(cp, 1);
      CopMove16(cp, bplcon2, p1);
      CopWait(cp, vp, HP(O2) + 2);
      CopSpriteSetHP(cp, 2);
      CopMove16(cp, bplcon2, p0);
      CopWait(cp, vp, HP(O3) + 2);
      CopSpriteSetHP(cp, 3);
      CopMove16(cp, bplcon2, p1);
      CopWait(cp, vp, HP(O4) + 2);
      CopSpriteSetHP(cp, 0);
      CopMove16(cp, bplcon2, p0);
    }
  }

  CopEnd(cp);
}

static void UpdateBarState(StateT *state) {
  short w = (bar_width - WIDTH) / 2;
  short f = frameCount * 16;
  short bx = w + normfx(SIN(f) * w);

  {
    CopInsT *ins = state->bar;

    short offset = (bx >> 3) & -2;
    short shift = ~bx & 15;

    CopInsSet32(&ins[0], bar.planes[0] + offset);
    CopInsSet32(&ins[2], bar.planes[1] + offset);
    CopInsSet32(&ins[4], bar.planes[2] + offset);
    CopInsSet32(&ins[6], bar.planes[3] + offset);
    CopInsSet16(&ins[8], (shift << 4) | shift);
  }

  {
    CopInsT **insp = state->bar_change;
    short shift, offset, bplmod, bx_prev, i;

    for (i = 0; i < BARS; i++) {
      CopInsT *ins = *insp++;

      f += SIN_HALF_PI;
      bx_prev = bx;
      bx = w + normfx(SIN(f) * w);

      shift = ~bx & 15;
      offset = (bx & -16) - (bx_prev & -16);
      bplmod = (bar_bplmod - bar_bplSize) + (offset >> 3);

      CopInsSet16(&ins[0], (shift << 4) | shift);
      CopInsSet16(&ins[1], bplmod);
      CopInsSet16(&ins[2], bplmod);
    }
  }
}

#define HPOFF(x) HP(x + 32)

static void UpdateStripeState(StateT *state) {
  static const char offset[STRIPES] = {
    HPOFF(O0), HPOFF(O1), HPOFF(O2), HPOFF(O3), HPOFF(O4) };
  int i;

  for (i = 0; i < STRIPES; i++) {
    u_char phase = StripePhase[i];
    CopInsT **stripesp = state->stripes;
    short hp_off = offset[i];
    short j;

    StripePhase[i] += StripePhaseIncr[i];

    for (j = 0; j < HEIGHT; j++) {
      CopInsT *ins = *stripesp++;
      short hp = sintab8[phase & 0xff] + hp_off;
      CopInsSet16(&ins[i * 4 + 0], hp);
      CopInsSet16(&ins[i * 4 + 1], hp + 8);
      phase += 2;
    }
  }
}

static void MakeSinTab8(void) {
  int i, j;

  for (i = 0, j = 0; i < 256; i++, j += 16)
    sintab8[i] = sintab[j] >> 10;
}

#define COPLIST_SIZE (HEIGHT * 22 + 100)

static void Init(void) {
  MakeSinTab8();

  SetupDisplayWindow(MODE_LORES, X(16), Y(0), WIDTH, HEIGHT);
  SetupBitplaneFetch(MODE_LORES, X(0), WIDTH + 16);
  SetupMode(MODE_LORES, DEPTH);
  LoadPalette(&bar_pal, 0);
  LoadPalette(&stripes_pal, 16);

  /* Place sprites 0-3 above playfield, and 4-7 below playfield. */
  custom->bplcon2 = BPLCON2_PF2PRI | BPLCON2_PF2P1 | BPLCON2_PF1P1;

  SpriteUpdatePos(&stripes0, X(0), Y(0));
  SpriteUpdatePos(&stripes1, X(0), Y(0));
  SpriteUpdatePos(&stripes2, X(0), Y(0));
  SpriteUpdatePos(&stripes3, X(0), Y(0));

  cp[0] = NewCopList(COPLIST_SIZE);
  cp[1] = NewCopList(COPLIST_SIZE);

  MakeCopperList(cp[0], &state[0]);
  MakeCopperList(cp[1], &state[1]);
  CopListActivate(cp[0]);

  Log("CopperList: %ld instructions left\n",
      COPLIST_SIZE - (cp[0]->curr - cp[0]->entry));
  EnableDMA(DMAF_RASTER|DMAF_SPRITE);
}

static void Kill(void) {
  DisableDMA(DMAF_RASTER|DMAF_SPRITE);
  DeleteCopList(cp[0]);
  DeleteCopList(cp[1]);
}

PROFILE(UpdateStripeState);

static void Render(void) {
  UpdateBarState(&state[active]);

  ProfilerStart(UpdateStripeState);
  UpdateStripeState(&state[active]);
  ProfilerStop(UpdateStripeState);

  CopListRun(cp[active]);
  TaskWaitVBlank();
  active ^= 1;
}

EFFECT(weave, NULL, NULL, Init, Kill, Render);
