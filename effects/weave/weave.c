#include "effect.h"

#include "custom.h"
#include "copper.h"
#include "blitter.h"
#include "bitmap.h"
#include "sprite.h"
#include "fx.h"

#include "data/bar.c"
#include "data/stripes.c"
#include "data/stripe-up.c"
#include "data/stripe-up-2.c"
#include "data/stripe-down.c"
#include "data/stripe-down-2.c"

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
 * |  down   |   down  |    up   |    up   |
 * +---------+---------+---------+---------+
 * |   0-1   |   4-5   |   2-3   |   6-7   |
 * |  above  |  below  |  above  |  below  |
 * +---------+---------+---------+---------+
 * |   4-5   |   0-1   |   6-7   |   2-3   |
 * |  below  |  above  |  below  |  above  |
 * +---------+---------+---------+---------+
 */

#define S0 0
#define S1 2
#define S2 1
#define S3 3

#define O0 0
#define O1 56
#define O2 112
#define O3 172
#define O4 224

static const u_short *stripe_pal[4] = {
  &stripe_down_2_pal.colors[1],
  &stripe_up_pal.colors[1],
  &stripe_down_pal.colors[1],
  &stripe_up_2_pal.colors[1],
};

static SprDataT *spriteA[8] = {
  &stripes0_sprdat, &stripes1_sprdat, /* S0 */
  &stripes0_sprdat, &stripes1_sprdat, /* S1 */
  &stripes2_sprdat, &stripes3_sprdat, /* S2 */
  &stripes2_sprdat, &stripes3_sprdat, /* S3 */
};

static SprDataT *spriteB[8] = {
  &stripes2_sprdat, &stripes3_sprdat, /* S2 */
  &stripes2_sprdat, &stripes3_sprdat, /* S3 */
  &stripes0_sprdat, &stripes1_sprdat, /* S0 */
  &stripes0_sprdat, &stripes1_sprdat, /* S1 */
};

static CopListT *cp0, *cp1;
static char sintab8[256];

#define STRIPES 5
#define BARS 4

static short StripePhase[STRIPES] = { 4, 2, 3, 8, 12 };
static short StripePhaseIncr[STRIPES] = { 7, -10, 12, -6, 14 };
static short BarOffset[STRIPES];

static inline void SetupBarBitplanes(CopListT *cp, short n) {
  short offset = (BarOffset[n] >> 3) & -2;
  short shift = 15 - (BarOffset[n] & 15);
  int i;

  for (i = 0; i < 4; i++)
    CopMove32(cp, bplpt[i], bar.planes[i] + offset);

  CopMove16(cp, bplcon1, (shift << 4) | shift);
  CopMove16(cp, bpl1mod, -WIDTH / 8 - 2);
  CopMove16(cp, bpl2mod, -WIDTH / 8 - 2);
}

static inline void SetupSpriteA(CopListT *cp, u_int y) {
  int i;

  for (i = 0; i < 8; i++)
    CopMove32(cp, sprpt[i], spriteA[i]->data[y]);
}

static inline void SetupSpriteB(CopListT *cp, u_int y) {
  int i;

  for (i = 0; i < 8; i++)
    CopMove32(cp, sprpt[i], spriteB[i]->data[y]);
}

static inline void CopLoadSprPal(CopListT *cp, const u_short *col, u_int i) {
  CopMove16(cp, color[i+1], col[0]);
  CopMove16(cp, color[i+2], col[1]);
  CopMove16(cp, color[i+3], col[2]);
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
  short *offset = (short *)StripeOffset;
  short y;
  int i;

  CopInit(cp);
  SetupBarBitplanes(cp, 0);

  for (i = 0; i < 8; i++)
    CopMove32(cp, sprpt[i], spriteA[i]);

  for (y = 0; y < HEIGHT; y++) {
    short vp = Y(y);

    CopWaitSafe(cp, vp, 0);

    if ((y & 3) == 0) {
      short mod_y = y & 63;

      if (mod_y == 0) {
        if (y & 64) {
          SetupSpriteB(cp, y);
          CopLoadSprPal(cp, stripe_pal[0], 24);
          CopLoadSprPal(cp, stripe_pal[1], 28);
          CopLoadSprPal(cp, stripe_pal[2], 16);
          CopLoadSprPal(cp, stripe_pal[3], 20);
        } else {
          SetupSpriteA(cp, y);
          CopLoadSprPal(cp, stripe_pal[0], 16);
          CopLoadSprPal(cp, stripe_pal[1], 20);
          CopLoadSprPal(cp, stripe_pal[2], 24);
          CopLoadSprPal(cp, stripe_pal[3], 28);
        }
      } else if (mod_y == 16) {
        CopMove16(cp, bpl1mod, (bar_width - WIDTH) / 8 - 2);
        CopMove16(cp, bpl2mod, (bar_width - WIDTH) / 8 - 2);
      } else if (mod_y == 48) {
        SetupBarBitplanes(cp, 1 + (y >> 6));
      }

      {
        CopInsT *ins = cp->curr;

        if (y & 64) {
          ChangeStripePosition(ins, S1, *offset++);
          ChangeStripePosition(ins, S0, *offset++);
          ChangeStripePosition(ins, S3, *offset++);
          ChangeStripePosition(ins, S2, *offset++);
          CopInsWait(ins, vp, X(O4));
          ChangeStripePosition(ins, S1, *offset++);
        } else {
          ChangeStripePosition(ins, S0, *offset++);
          ChangeStripePosition(ins, S1, *offset++);
          ChangeStripePosition(ins, S2, *offset++);
          ChangeStripePosition(ins, S3, *offset++);
          CopInsWait(ins, vp, X(O4));
          ChangeStripePosition(ins, S0, *offset++);
        }

        cp->curr = ins;
      }
    } else {
      CopInsT *ins = cp->curr;

      if (y & 64) {
        ChangeStripePosition(ins, S1, offset[0]);
        CopInsWait(ins, vp, X(O4));
        ChangeStripePosition(ins, S1, offset[4]);
      } else {
        ChangeStripePosition(ins, S0, offset[0]);
        CopInsWait(ins, vp, X(O4));
        ChangeStripePosition(ins, S0, offset[4]);
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
  {
    short w = (bar.width - WIDTH) / 2;
    short i = 0;

    CalculateStripeOffsets();

    for (i = 0; i < BARS; i++)
      BarOffset[i] = normfx(SIN(frameCount * 16 + i * SIN_HALF_PI) * w) + w;
  }
  ProfilerStop(Prepare);

  ProfilerStart(MakeCopperList);
  MakeCopperList(cp1);
  ProfilerStop(MakeCopperList);

  CopListRun(cp1);
  TaskWaitVBlank();
  swapr(cp0, cp1);
}

EFFECT(weave, NULL, NULL, Init, Kill, Render);
