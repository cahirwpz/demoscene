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

#define O1 56
#define O2 112
#define O3 172
#define O4 224

static const PaletteT *stripe_pal[4] = {
  &stripe_down_2_pal,
  &stripe_up_pal,
  &stripe_down_pal,
  &stripe_up_2_pal,
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

#define STRIPES 5
#define BARS 4

static inline void ChangeStripePosition(CopListT *cp, short n, short hp) {
  CopMove16(cp, spr[n + 0].pos, hp);
  CopMove16(cp, spr[n + 1].pos, hp + 8);
}

#define SL4(x) ((x) << 4)

static u_short StripePhase[STRIPES] = {
  SL4(1024), SL4(512), SL4(1536), SL4(2048), SL4(3072)};
static short StripePhaseIncr[STRIPES] = {
  SL4(7), SL4(-10), SL4(12), SL4(-6), SL4(14)};
static short BarOffset[STRIPES];

static inline void SetupBarBitplanes(CopListT *cp, short n) {
  short offset = (BarOffset[n] >> 3) & -2;
  short shift = 15 - (BarOffset[n] & 15);
  short i;

  for (i = 0; i < 4; i++)
    CopMove32(cp, bplpt[i], bar.planes[i] + offset);

  CopMove16(cp, bplcon1, (shift << 4) | shift);
  CopMove16(cp, bpl1mod, -WIDTH / 8 - 2);
  CopMove16(cp, bpl2mod, -WIDTH / 8 - 2);
}

static inline void SetupSpriteA(CopListT *cp, u_int y) {
  int i;

  for (i = 0; i < 8; i++) {
    u_int *dat = (u_int *)spriteA[i]->data;
    CopMove32(cp, sprpt[i], dat + y);
  }
}

static inline void SetupSpriteB(CopListT *cp, u_int y) {
  int i;

  for (i = 0; i < 8; i++) {
    u_int *dat = (u_int *)spriteB[i]->data;
    CopMove32(cp, sprpt[i], dat + y);
  }
}

static inline void CopLoadSprPal(CopListT *cp, const PaletteT *pal, u_int i) {
  const u_short *col = &pal->colors[1];
  CopMove16(cp, color[i+1], *col++);
  CopMove16(cp, color[i+2], *col++);
  CopMove16(cp, color[i+3], *col++);
}

static inline short SIN8(short a) {
  short res;
  a &= SIN_MASK << 1;
  asm("moveb (%2,%1:w),%0\n\t"
      "extw %0\n\t"
      : "=r" (res)
      : "d" (a), "a" (sintab)
      : "1");
  return res;
}

static void MakeCopperList(CopListT *cp) {
  short offset[STRIPES];
  short y, i;

  CopInit(cp);
  SetupBarBitplanes(cp, 0);

  for (i = 0; i < 8; i++)
    CopMove32(cp, sprpt[i], spriteA[i]);

  for (y = 0; y < HEIGHT; y++) {
    CopWaitSafe(cp, Y(y), 0);

    if ((y & 3) == 0) {
      short mod_y = y & 63;

      if (mod_y == 0) {
        if (y & 64) {
          SetupSpriteB(cp, y);
          CopLoadSprPal(cp, stripe_pal[2], 16);
          CopLoadSprPal(cp, stripe_pal[3], 20);
          CopLoadSprPal(cp, stripe_pal[0], 24);
          CopLoadSprPal(cp, stripe_pal[1], 28);
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
        short *phasep = StripePhase;
        short *offsetp = offset;
        short yp = SL4(y << 2);

        for (i = 0; i < STRIPES; i++) {
          short phase = (*phasep++) + yp;
          short y = SIN8(phase) >> 1;
          *offsetp++ = (short)X(y + 32) >> 1;
        }
      }

      if (y & 64) {
        ChangeStripePosition(cp, S1 * 2, offset[0]);
        ChangeStripePosition(cp, S0 * 2, offset[1] + O1 / 2);
        ChangeStripePosition(cp, S3 * 2, offset[2] + O2 / 2);
        ChangeStripePosition(cp, S2 * 2, offset[3] + O3 / 2);
        CopWait(cp, Y(y), X(O4));
        ChangeStripePosition(cp, S1 * 2, offset[4] + O4 / 2);
      } else {
        ChangeStripePosition(cp, S0 * 2, offset[0]);
        ChangeStripePosition(cp, S1 * 2, offset[1] + O1 / 2);
        ChangeStripePosition(cp, S2 * 2, offset[2] + O2 / 2);
        ChangeStripePosition(cp, S3 * 2, offset[3] + O3 / 2);
        CopWait(cp, Y(y), X(O4));
        ChangeStripePosition(cp, S0 * 2, offset[4] + O4 / 2);
      }
    } else {
      if (y & 64) {
        ChangeStripePosition(cp, S1 * 2, offset[0]);
        CopWait(cp, Y(y), X(O4));
        ChangeStripePosition(cp, S1 * 2, offset[4] + O4 / 2);
      } else {
        ChangeStripePosition(cp, S0 * 2, offset[0]);
        CopWait(cp, Y(y), X(O4));
        ChangeStripePosition(cp, S0 * 2, offset[4] + O4 / 2);
      }
    }
  }

  CopEnd(cp);
}

static void Init(void) {
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

PROFILE(MakeCopperList);

static void Render(void) {
  short w = (bar.width - WIDTH) / 2;
  short i = 0;

  for (i = 0; i < STRIPES; i++)
    StripePhase[i] += StripePhaseIncr[i];

  for (i = 0; i < BARS; i++)
    BarOffset[i] = normfx(SIN(frameCount * 16 + i * SIN_HALF_PI) * w) + w;

  ProfilerStart(MakeCopperList);
  MakeCopperList(cp1);
  ProfilerStop(MakeCopperList);

  CopListRun(cp1);
  TaskWaitVBlank();
  swapr(cp0, cp1);
}

EFFECT(weave, NULL, NULL, Init, Kill, Render);
