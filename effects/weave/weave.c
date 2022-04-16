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

/*
 * Sprite priorities:
 *  - are fixed by the hardware
 *  - 0 is the highest, 7 is the lowest
 *  - bitplanes were configured to be placed between sprites 4 and 5
 *
 * Sprites are displayed on screen as follows:
 *
 * +---------+---------+---------+---------+
 * | S0(0-1) | S1(4-5) | S2(2-3) | S3(6-7) |
 * |  above  |  below  |  above  |  below  |
 * +---------+---------+---------+---------+
 * | S1(4-5) | S0(0-1) | S3(6-7) | S1(2-3) |
 * |  below  |  above  |  below  |  above  |
 * +---------+---------+---------+---------+
 */

#define S0 0
#define S1 2
#define S2 1
#define S3 3

#define DAT0A (&stripes0_sprdat) /* S0, up */
#define DAT1A (&stripes1_sprdat)
#define DAT2A (&stripes0_sprdat) /* S1, up */
#define DAT3A (&stripes1_sprdat)
#define DAT4A (&stripes2_sprdat) /* S2, down */
#define DAT5A (&stripes3_sprdat)
#define DAT6A (&stripes2_sprdat) /* S3, down */
#define DAT7A (&stripes3_sprdat)

#define PAL0A (&stripes_pal.colors[1])  /* S0,  1.. 3, warm dull  */
#define PAL1A (&stripes_pal.colors[9])  /* S1,  9..11, green */
#define PAL2A (&stripes_pal.colors[5])  /* S2,  5.. 7, warm vivid  */
#define PAL3A (&stripes_pal.colors[13]) /* S3, 13..15, magenta */

#define DAT0B DAT4A
#define DAT1B DAT5A
#define DAT2B DAT6A
#define DAT3B DAT7A
#define DAT4B DAT0A
#define DAT5B DAT1A
#define DAT6B DAT2A
#define DAT7B DAT3A

#define PAL0B PAL2A
#define PAL1B PAL3A
#define PAL2B PAL0A
#define PAL3B PAL1A

#define O0 0
#define O1 56
#define O2 112
#define O3 172
#define O4 224

static CopListT *cp0, *cp1;
static char sintab8[256];

#define STRIPES 5
#define BARS 4

static short StripePhase[STRIPES] = { 4, 2, 3, 8, 12 };
static short StripePhaseIncr[STRIPES] = { 7, -10, 12, -6, 14 };
static short BarOffset[STRIPES];

static inline void SetupBarBitplanes(CopListT *cp, short x) {
  CopInsT *ins = cp->curr;

  short offset = (x >> 3) & -2;
  short shift = ~x & 15;

  CopInsMove32(ins, bplpt[0], bar.planes[0] + offset);
  CopInsMove32(ins, bplpt[1], bar.planes[1] + offset);
  CopInsMove32(ins, bplpt[2], bar.planes[2] + offset);
  CopInsMove32(ins, bplpt[3], bar.planes[3] + offset);

  CopInsMove16(ins, bplcon1, (shift << 4) | shift);

  /* Move back bitplane pointers to repeat the line. */
  CopInsMove16(ins, bpl1mod, -WIDTH / 8 - 2);
  CopInsMove16(ins, bpl2mod, -WIDTH / 8 - 2);

  cp->curr = ins;
}

/* Move back bitplane pointers to the beginning of bitmap. Take into
 * account new bitmap offset and shifter configuration for next bar. */
static inline void SetupBarFetcher(CopListT *cp, short prev_x, short x) {
  CopInsT *ins = cp->curr;

  short shift = ~x & 15;
  short offset = (x & -16) - (prev_x & -16);
  short bplmod = (bar_bplmod - bar_bplSize) + (offset >> 3);

  CopInsMove16(ins, bplcon1, (shift << 4) | shift);
  CopInsMove16(ins, bpl1mod, bplmod);
  CopInsMove16(ins, bpl2mod, bplmod);

  cp->curr = ins;
}

#define CopLoadSprPal(ins, col, i)                                             \
  ins = _CopLoadSprPal(ins, col, i)

static inline CopInsT *_CopLoadSprPal(CopInsT *ins, const u_short *col, int i) {
  CopInsMove16(ins, color[i+1], col[0]);
  CopInsMove16(ins, color[i+2], col[1]);
  CopInsMove16(ins, color[i+3], col[2]);
  return ins;
}

static inline void SetupSpriteA(CopListT *cp, int y) {
  CopInsT *ins = cp->curr;
  CopInsMove32(ins, sprpt[0], DAT0A->data[y]);
  CopInsMove32(ins, sprpt[1], DAT1A->data[y]);
  CopInsMove32(ins, sprpt[2], DAT2A->data[y]);
  CopInsMove32(ins, sprpt[3], DAT3A->data[y]);
  CopInsMove32(ins, sprpt[4], DAT4A->data[y]);
  CopInsMove32(ins, sprpt[5], DAT5A->data[y]);
  CopInsMove32(ins, sprpt[6], DAT6A->data[y]);
  CopInsMove32(ins, sprpt[7], DAT7A->data[y]);
  CopLoadSprPal(ins, PAL0A, 16);
  CopLoadSprPal(ins, PAL1A, 20);
  CopLoadSprPal(ins, PAL2A, 24);
  CopLoadSprPal(ins, PAL3A, 28);
  cp->curr = ins;
}

static inline void SetupSpriteB(CopListT *cp, int y) {
  CopInsT *ins = cp->curr;
  CopInsMove32(ins, sprpt[0], DAT0B->data[y]);
  CopInsMove32(ins, sprpt[1], DAT1B->data[y]);
  CopInsMove32(ins, sprpt[2], DAT2B->data[y]);
  CopInsMove32(ins, sprpt[3], DAT3B->data[y]);
  CopInsMove32(ins, sprpt[4], DAT4B->data[y]);
  CopInsMove32(ins, sprpt[5], DAT5B->data[y]);
  CopInsMove32(ins, sprpt[6], DAT6B->data[y]);
  CopInsMove32(ins, sprpt[7], DAT7B->data[y]);
  CopLoadSprPal(ins, PAL0B, 16);
  CopLoadSprPal(ins, PAL1B, 20);
  CopLoadSprPal(ins, PAL2B, 24);
  CopLoadSprPal(ins, PAL3B, 28);
  cp->curr = ins;
}

#define ChangeStripePosition(ins, n, hp)                                       \
  ins = _ChangeStripePosition(ins, n, hp)

static inline CopInsT *_ChangeStripePosition(CopInsT *ins, short n, short hp) {
  CopInsMove16(ins, spr[n * 2 + 0].pos, hp);
  CopInsMove16(ins, spr[n * 2 + 1].pos, hp + 8);
  return ins;
}

static short StripeOffset[HEIGHT / 4 + 1][STRIPES];

static void MakeCopperList(CopListT *cp) {
  short *stripe_hpos = (short *)StripeOffset;
  short *bar_x = BarOffset;
  short y;

  CopInit(cp);
  SetupBarBitplanes(cp, bar_x[0]);
  SetupSpriteA(cp, -1);

  for (y = 0; y < HEIGHT; y++) {
    short vp = Y(y);
    short my = y & 63;

    CopWaitSafe(cp, vp, 0);

    /* With current solution bitplane setup takes at most 3 copper move
     * instructions (bpl1mod, bpl2mod, bplcon1) per raster line. */
    if ((y & 15) == 0) {
      if (my == 0) {
        if (y & 64) {
          SetupSpriteB(cp, y);
        } else {
          SetupSpriteA(cp, y);
        }
      } else if (my == 16) {
        /* Advance bitplane pointers to display consecutive lines. */
        CopMove16(cp, bpl1mod, bar_bplmod);
        CopMove16(cp, bpl2mod, bar_bplmod);
      } else if (my == 48) {
        SetupBarFetcher(cp, bar_x[0], bar_x[1]);
        bar_x++;
      }
    }

    if (my == 49) {
      /* Move back bitplane pointers to repeat the line. */
      CopMove16(cp, bpl1mod, -WIDTH / 8 - 2);
      CopMove16(cp, bpl2mod, -WIDTH / 8 - 2);
    }

    {
      CopInsT *ins = cp->curr;

      if ((y & 3) == 0) {
        if (y & 64) {
          ChangeStripePosition(ins, S1, *stripe_hpos++);
          ChangeStripePosition(ins, S0, *stripe_hpos++);
          ChangeStripePosition(ins, S3, *stripe_hpos++);
          ChangeStripePosition(ins, S2, *stripe_hpos++);
          CopInsWait(ins, vp, X(O4));
          ChangeStripePosition(ins, S1, *stripe_hpos++);
        } else {
          ChangeStripePosition(ins, S0, *stripe_hpos++);
          ChangeStripePosition(ins, S1, *stripe_hpos++);
          ChangeStripePosition(ins, S2, *stripe_hpos++);
          ChangeStripePosition(ins, S3, *stripe_hpos++);
          CopInsWait(ins, vp, X(O4));
          ChangeStripePosition(ins, S0, *stripe_hpos++);
        }
      } else {
        if (y & 64) {
          ChangeStripePosition(ins, S1, stripe_hpos[0]);
          CopInsWait(ins, vp, X(O4));
          ChangeStripePosition(ins, S1, stripe_hpos[4]);
        } else {
          ChangeStripePosition(ins, S0, stripe_hpos[0]);
          CopInsWait(ins, vp, X(O4));
          ChangeStripePosition(ins, S0, stripe_hpos[4]);
        }
      }

      cp->curr = ins;
    }
  }

  CopEnd(cp);
}

#define HPOFF(x) (X(x + 32) / 2)

static void CalculateStripeOffsets(void) {
  static const short offset[STRIPES] = {
    HPOFF(O0), HPOFF(O1), HPOFF(O2), HPOFF(O3), HPOFF(O4) };
  int i;

  for (i = 0; i < STRIPES; i++) {
    short *stripes = (short *)StripeOffset + i;
    u_char phase = StripePhase[i];
    short hp_off = offset[i];
    short j;

    for (j = 0; j <= HEIGHT / 4; j++) {
      short hp = sintab8[phase & 0xff];
      *stripes++ = hp + hp_off;
      stripes += STRIPES - 1;
      phase += 8;
    }

    StripePhase[i] += StripePhaseIncr[i];
  }
}

static void CalculateBarOffsets(void) {
  short w = (bar_width - WIDTH) / 2;
  short f = frameCount * 16;
  int i = 0;

  for (i = 0; i < BARS; i++, f += SIN_HALF_PI)
    BarOffset[i] = w + normfx(SIN(f) * w);
}

static void MakeSinTab8(void) {
  int i, j;

  for (i = 0, j = 0; i < 256; i++, j += 16)
    sintab8[i] = sintab[j] >> 10;
}

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

  cp0 = NewCopList(HEIGHT * 16 + 100);
  cp1 = NewCopList(HEIGHT * 16 + 100);

  MakeCopperList(cp0);
  MakeCopperList(cp1);
  CopListActivate(cp0);
  EnableDMA(DMAF_RASTER|DMAF_SPRITE);
}

static void Kill(void) {
  DisableDMA(DMAF_RASTER|DMAF_SPRITE);
  DeleteCopList(cp0);
  DeleteCopList(cp1);
}

PROFILE(Prepare);
PROFILE(MakeCopperList);

static void Render(void) {
  ProfilerStart(Prepare);
  CalculateStripeOffsets();
  CalculateBarOffsets();
  ProfilerStop(Prepare);

  ProfilerStart(MakeCopperList);
  MakeCopperList(cp1);
  ProfilerStop(MakeCopperList);

  CopListRun(cp1);
  TaskWaitVBlank();
  swapr(cp0, cp1);
}

EFFECT(weave, NULL, NULL, Init, Kill, Render);
