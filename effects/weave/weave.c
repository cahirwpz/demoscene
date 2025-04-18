#include "effect.h"

#include "custom.h"
#include "copper.h"
#include "blitter.h"
#include "bitmap.h"
#include "sprite.h"
#include "fx.h"

#include "data/bar.c"
#include "data/bar-2.c"
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
  CopInsPairT *sprite;
  /* at the beginning: 4 bitplane pointers and bplcon1 */
  CopInsPairT *bar;
  /* for each bar moves to bplcon1, bpl1mod and bpl2mod */
  CopInsT *bar_change[4];
  /* for each line five horizontal positions */
  CopInsT *stripes[HEIGHT];
} StateT;

static int active = 1;
static CopListT *cp[2];
static short sintab8[128];
static StateT state[2];

#define STRIPES 5
#define BARS 4

/* These numbers must be odd due to optimizations. */
static u_char StripePhase[STRIPES] = { 4, 24, 16, 8, 12 };
static char StripePhaseIncr[STRIPES] = { 8, -10, 14, -6, 6 };

static inline void CopSpriteSetHP(CopListT *cp, short n) {
  CopMove16(cp, spr[n * 2 + 0].pos, 0);
  CopMove16(cp, spr[n * 2 + 1].pos, 0);
}

#define COPLIST_SIZE (HEIGHT * 22 + 100)

static CopListT *MakeCopperList(StateT *state) {
  CopListT *cp = NewCopList(COPLIST_SIZE);
  short b, y;

  /* Setup initial bitplane pointers. */
  state->bar = CopMove32(cp, bplpt[0], NULL);
  CopMove32(cp, bplpt[1], NULL);
  CopMove32(cp, bplpt[2], NULL);
  CopMove32(cp, bplpt[3], NULL);
  CopMove16(cp, bplcon1, 0);

  /* Move back bitplane pointers to repeat the line. */
  CopMove16(cp, bpl1mod, -WIDTH / 8 - 2);
  CopMove16(cp, bpl2mod, -WIDTH / 8 - 2);

  /* Load default sprite settings */
  CopMove32(cp, sprpt[0], &stripes0_sprdat); /* up */
  CopMove32(cp, sprpt[1], &stripes1_sprdat);
  CopMove32(cp, sprpt[2], &stripes2_sprdat); /* down */
  CopMove32(cp, sprpt[3], &stripes3_sprdat);
  CopMove32(cp, sprpt[4], &stripes0_sprdat); /* up */
  CopMove32(cp, sprpt[5], &stripes1_sprdat);
  CopMove32(cp, sprpt[6], &stripes2_sprdat); /* down */
  CopMove32(cp, sprpt[7], &stripes3_sprdat);

  CopWait(cp, Y(-1), 0);
  
  state->sprite = CopMove32(cp, sprpt[0], stripes0_sprdat.data); /* up */
  CopMove32(cp, sprpt[1], stripes1_sprdat.data);
  CopMove32(cp, sprpt[2], stripes2_sprdat.data); /* down */
  CopMove32(cp, sprpt[3], stripes3_sprdat.data);
  CopMove32(cp, sprpt[4], stripes0_sprdat.data); /* up */
  CopMove32(cp, sprpt[5], stripes1_sprdat.data);
  CopMove32(cp, sprpt[6], stripes2_sprdat.data); /* down */
  CopMove32(cp, sprpt[7], stripes3_sprdat.data);

  for (y = 0, b = 0; y < HEIGHT; y++) {
    short vp = Y(y);
    short my = y & 63;

    CopWaitSafe(cp, vp, 0);

    /* With current solution bitplane setup takes at most 3 copper move
     * instructions (bpl1mod, bpl2mod, bplcon1) per raster line. */
    if (my == 8) {
      if (y & 64) {
        CopLoadColors(cp, bar_colors, 0);
      } else {
        CopLoadColors(cp, bar2_colors, 0);
      }
    } else if (my == 16) {
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

  return CopListFinish(cp);
}

static void UpdateBarState(StateT *state) {
  short w = (bar_width - WIDTH) / 2;
  short f = frameCount * 16;
  short bx = w + normfx(SIN(f) * w);

  {
    CopInsPairT *ins = state->bar;

    short offset = (bx >> 3) & -2;
    short shift = ~bx & 15;

    CopInsSet32(&ins[0], bar.planes[0] + offset);
    CopInsSet32(&ins[1], bar.planes[1] + offset);
    CopInsSet32(&ins[2], bar.planes[2] + offset);
    CopInsSet32(&ins[3], bar.planes[3] + offset);
    CopInsSet16((CopInsT *)&ins[4], (shift << 4) | shift);
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
      bplmod = bar_bplmod + (offset >> 3);

      CopInsSet16(&ins[0], (shift << 4) | shift);
      CopInsSet16(&ins[1], bplmod);
      CopInsSet16(&ins[2], bplmod);
    }
  }
}

static void UpdateSpriteState(StateT *state) {
  CopInsPairT *ins = state->sprite;
  int fu = frameCount & 63;
  int fd = (~frameCount) & 63;

  CopInsSet32(ins++, stripes0_sprdat.data + fu); /* up */
  CopInsSet32(ins++, stripes1_sprdat.data + fu);
  CopInsSet32(ins++, stripes2_sprdat.data + fd); /* down */
  CopInsSet32(ins++, stripes3_sprdat.data + fd);
  CopInsSet32(ins++, stripes0_sprdat.data + fu); /* up */
  CopInsSet32(ins++, stripes1_sprdat.data + fu);
  CopInsSet32(ins++, stripes2_sprdat.data + fd); /* down */
  CopInsSet32(ins++, stripes3_sprdat.data + fd);
}

#define HPOFF(x) HP(x + 32)

static void UpdateStripeState(StateT *state) {
  static const char offset[STRIPES] = {
    HPOFF(O0), HPOFF(O1), HPOFF(O2), HPOFF(O3), HPOFF(O4) };
  u_char *phasep = StripePhase;
  char *incrp = StripePhaseIncr;
  const char *offsetp = offset;
  short i;

  for (i = 0; i < STRIPES * 4; i += 4) {
    u_int phase = (u_char)*phasep;
    CopInsT **stripesp = state->stripes;
    short hp_off = *offsetp++;
    short n = HEIGHT / 8 - 1;

    (*phasep++) += (*incrp++);

    do {
      CopInsT *ins;
      short hp;

#define BODY()                                          \
      ins = *stripesp++;                                \
      asm ("movew (%2,%1:w),%0\n\t"                     \
           "addqb #2,%1\n\t"                            \
           "addw  %3,%0"                                \
           : "=d" (hp), "+d" (phase)                    \
           : "a" (sintab8), "d" (hp_off));              \
      CopInsSet16(&ins[i + 0], hp);                     \
      CopInsSet16(&ins[i + 1], hp + 8);

      BODY(); BODY(); BODY(); BODY();
      BODY(); BODY(); BODY(); BODY();
    } while (--n != -1);
  }
}

static void MakeSinTab8(void) {
  int i, j;

  for (i = 0, j = 0; i < 128; i++, j += 32)
    sintab8[i] = (sintab[j] + 512) >> 10;
}

static void Init(void) {
  MakeSinTab8();

  SetupDisplayWindow(MODE_LORES, X(16), Y(0), WIDTH, HEIGHT);
  SetupBitplaneFetch(MODE_LORES, X(0), WIDTH + 16);
  SetupMode(MODE_LORES, DEPTH);
  LoadColors(bar_colors, 0);
  LoadColors(stripes_colors, 16);

  /* Place sprites 0-3 above playfield, and 4-7 below playfield. */
  custom->bplcon2 = BPLCON2_PF2PRI | BPLCON2_PF2P1 | BPLCON2_PF1P1;

  SpriteUpdatePos(&stripes[0], X(0), Y(0));
  SpriteUpdatePos(&stripes[1], X(0), Y(0));
  SpriteUpdatePos(&stripes[2], X(0), Y(0));
  SpriteUpdatePos(&stripes[3], X(0), Y(0));

  cp[0] = MakeCopperList(&state[0]);
  cp[1] = MakeCopperList(&state[1]);
  CopListActivate(cp[0]);

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
  UpdateSpriteState(&state[active]);

  ProfilerStart(UpdateStripeState);
  UpdateStripeState(&state[active]);
  ProfilerStop(UpdateStripeState);

  CopListRun(cp[active]);
  WaitVBlank();
  active ^= 1;
}

EFFECT(Weave, NULL, NULL, Init, Kill, Render, NULL);
