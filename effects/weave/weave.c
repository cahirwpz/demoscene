#include "effect.h"

#include "custom.h"
#include "copper.h"
#include "blitter.h"
#include "bitmap.h"
#include "sprite.h"
#include "fx.h"

#include "data/bar.c"
#include "data/stripe-up.c"
#include "data/stripe-up-2.c"
#include "data/stripe-down.c"
#include "data/stripe-down-2.c"

#define WIDTH (320 - 32)
#define HEIGHT 256
#define DEPTH 4
#define NSPRITES 8

static CopListT *cp0, *cp1;
static CopInsT *sprptr[8];

#define STRIPES 5
#define BARS 4

static inline void ChangeStripePosition(CopListT *cp, short n, short hp) {
  CopMove16(cp, spr[n + 0].pos, hp);
  CopMove16(cp, spr[n + 1].pos, hp + 8);
}

static short StripePhase[STRIPES] = {1024, 512, 1536, 2048, 3072};
static short StripePhaseIncr[STRIPES] = {7, -10, 12, -6, 14};
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

static void MakeCopperList(CopListT *cp) {
  short offset[STRIPES];
  short y, i;

  CopInit(cp);
  CopSetupDisplayWindow(cp, MODE_LORES, X(16), Y(0), WIDTH, HEIGHT);
  CopSetupBitplaneFetch(cp, MODE_LORES, X(0), WIDTH + 16);
  CopSetupMode(cp, MODE_LORES, DEPTH);
  CopMove16(cp, bplcon2, BPLCON2_PF2PRI | BPLCON2_PF2P1 | BPLCON2_PF1P1);
  CopLoadPal(cp, &bar_pal, 0);
  CopLoadPal(cp, &stripe_down_pal, 16);
  CopLoadPal(cp, &stripe_down_2_pal, 20);
  CopLoadPal(cp, &stripe_up_pal, 24);
  CopLoadPal(cp, &stripe_up_2_pal, 28);

  SetupBarBitplanes(cp, 0);
  CopSetupSprites(cp, sprptr);

  {
    short i;

    for (i = 0; i < 8; i++) {
      SpriteT *spr = (i & 2) ? stripe_up[i & 1] : stripe_down[i & 1];
      SpriteUpdatePos(spr, X(0), Y(0));
      CopInsSetSprite(sprptr[i], spr);
    }
  }

  for (y = 0; y < HEIGHT; y++) {
    CopWaitSafe(cp, Y(y), 0);

    if ((y & 3) == 0) {
      short mod_y = y & 63;

      if (mod_y == 0) {
        if (y & 64) {
          CopLoadPal(cp, &stripe_up_pal, 16);
          CopLoadPal(cp, &stripe_up_2_pal, 20);
          CopLoadPal(cp, &stripe_down_pal, 24);
          CopLoadPal(cp, &stripe_down_2_pal, 28);
        } else {
          CopLoadPal(cp, &stripe_down_pal, 16);
          CopLoadPal(cp, &stripe_down_2_pal, 20);
          CopLoadPal(cp, &stripe_up_pal, 24);
          CopLoadPal(cp, &stripe_up_2_pal, 28);
        }
      } else if (mod_y == 16) {
        CopMove16(cp, bpl1mod, (bar.width - WIDTH) / 8 - 2);
        CopMove16(cp, bpl2mod, (bar.width - WIDTH) / 8 - 2);
      } else if (mod_y == 48) {
        SetupBarBitplanes(cp, 1 + (y >> 6));
      }

      for (i = 0; i < STRIPES; i++) {
        short phase = StripePhase[i] + (y << 2);
        short y = normfx(SIN(phase << 3) << 3) + 32;
        offset[i] = X(y) / 2;
      }

      if (y & 64) {
        ChangeStripePosition(cp, 4, offset[0]);
        ChangeStripePosition(cp, 0, offset[1] + 56 / 2);
        ChangeStripePosition(cp, 6, offset[2] + 112 / 2);
        ChangeStripePosition(cp, 2, offset[3] + 172 / 2);
      } else {
        ChangeStripePosition(cp, 0, offset[0]);
        ChangeStripePosition(cp, 4, offset[1] + 56 / 2);
        ChangeStripePosition(cp, 2, offset[2] + 112 / 2);
        ChangeStripePosition(cp, 6, offset[3] + 172 / 2);
      }
    } else {
      ChangeStripePosition(cp, (y >> 4) & 4, offset[0]);
    }

    CopWait(cp, Y(y), X(224));
    ChangeStripePosition(cp, (y >> 4) & 4, offset[4] + 224 / 2);
  }

  CopEnd(cp);
}

static void Init(void) {
  SetupPlayfield(MODE_LORES, DEPTH, X(0), Y(0), WIDTH, HEIGHT);

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
