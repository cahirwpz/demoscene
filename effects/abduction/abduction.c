#include <effect.h>
#include <blitter.h>
#include <copper.h>
#include <sprite.h>
#include <system/memory.h>
#include <system/interrupt.h>
#include <common.h>
#include <2d.h>

#include "data/bkg.c"
#include "data/ufo.c"
#include "data/ring.c"
#include "data/coq.c"
#include "data/mid_beam.c"
#include "data/side_beam_l.c"
#include "data/side_beam_r.c"


#define WIDTH 320
#define HEIGHT 256
#define DEPTH 4

#define RING_W 56
#define RING_H 16

#define UFO_W 104
#define UFO_H 39


typedef enum phase {
  ABDUCT,
  RETRACT_BEAM,
  ESCAPE,
  END
} phaseE;


static BitmapT *screen[2];
static CopInsPairT *bplptr;
static CopInsPairT *sprptr;
static CopListT *cp;
static CopInsT *beam_pal_cp;
static short active = 0;

static phaseE phase = ABDUCT;
static short counter = 0;

static short ufo_pos = 25;
static short coq_pos = 255-24;
static short beam_pos[2] = {X(137), X(167)};

static short active_pal = 0;
static short beam_pal[4][7] = {
  {0xCEF, 0xCEF, 0xCEF, 0xF0F, 0x2AD, 0x079, 0x046},
  {0xDFF, 0xDFF, 0xDFF, 0xF0F, 0x3BE, 0x18A, 0x157},
  {0xFFF, 0xFFF, 0xFFF, 0xF0F, 0x4CF, 0x29B, 0x268},
  {0xDFF, 0xDFF, 0xDFF, 0xF0F, 0x3BE, 0x18A, 0x157},
};


static void DrawBackground(BitmapT *dst) {
  BitmapCopyArea(dst, 0, 0, &bkg, &((Area2D){0, 0, WIDTH, HEIGHT}));
}

static void DrawUfo(BitmapT *dst) {
  Area2D ufo_area = {0, 0, UFO_W, UFO_H};
  short h = ufo_pos;

  if (ufo_pos <= 0) {
    ufo_area.y = -ufo_pos;
    ufo_area.h = 39 + ufo_pos;
    h = 0;
  }

  BitmapCopyArea(dst, 108, h, &ufo, &ufo_area);
}

static void DrawRing(BitmapT *dst, short height) {
  Area2D ring_area = {0, 0, RING_W, RING_H};

  if (height < 64) {
    ring_area.y = 64 - height;
    ring_area.h = RING_H - (64 - height);
    height = 64;
  }

  if (height + RING_H >= HEIGHT - 16) {
    ring_area.h = HEIGHT - height - 16;
  }

  BitmapCopyArea(dst, 133, height, &ring, &ring_area);
}

static void ClearRing(BitmapT *dst, short height) {
  Area2D ring_area = {133, height, RING_W, RING_H};

  if (height < 64) {
    ring_area.y = 64 - height;
    ring_area.h = RING_H - (64 - height);
    height = 64;
  }

  if (height + RING_H >= HEIGHT - 16) {
    ring_area.h = HEIGHT - height - 16;
  }

  BitmapClearArea(dst, &ring_area);
}

static void SwitchBeamPal(void) {
  CopInsT *ins = beam_pal_cp;
  short i = 0;

  for (i = 0; i < 7; ++i) {
    CopInsSet16(ins, beam_pal[active_pal][i]);
    ++ins;
  }

  active_pal = (active_pal + 1) % 4;
}


static void Abduct(void) {
  static short mod = 1;
  short i = 0;

  for (i = 64+16; i <= 256 - 16; i += RING_H*2) {
    DrawRing(screen[active], i - mod);
  }

  if (counter % 2 == 0) {
    ++mod;
    if (mod >= RING_H*2) {
      mod = 1;
    }
  }

  if (counter % 3 == 0) {
    if (coq_pos > 32) {
      --coq_pos;
      SpriteUpdatePos(&coq,  X(152), Y(coq_pos));
    } else {
      phase = RETRACT_BEAM;
    }
  }

  if (counter % 8 == 0) {
    SwitchBeamPal();
  }
}

static void RetractBeam(void) {
  static unsigned short beam_gradient[15] = {
    0xcef,0xcdf,0xbce,0xacd,0x9bc,0x8ab,0x79a,0x689,
    0x578,0x478,0x367,0x356,0x245,0x144,0x034
  };
  static short idx = 0;
  static short h = 224;
  Area2D ring_area = {133, h, RING_W, RING_H};

  if (counter % 2 == 0) {
      active_pal = 0;
      beam_pal[0][0] = beam_gradient[idx];
      beam_pal[0][1] = beam_gradient[idx];
      beam_pal[0][2] = beam_gradient[idx];
      SwitchBeamPal();
      ++idx;

    SpriteUpdatePos(&side_beam_l, ++beam_pos[0], Y(56));
    SpriteUpdatePos(&side_beam_r, --beam_pos[1], Y(56));

    if (h >= 64) {
      BitmapClearArea(screen[0], &ring_area);
      BitmapClearArea(screen[1], &ring_area);
      h -= 16;
    }

    if (beam_pos[0] >= X(137+15)) {
      SpriteUpdatePos(&side_beam_l, 0, 0);
      SpriteUpdatePos(&side_beam_r, 0, 0);
      SpriteUpdatePos(&coq, 0, 0);
      phase = ESCAPE;
    }
  }
}

static void Escape(void) {
  Area2D ring_area = {132, ufo_pos + UFO_H - 2, RING_W, RING_H};

  BitmapClearArea(screen[0], &ring_area);
  BitmapClearArea(screen[1], &ring_area);

  --ufo_pos;
  DrawUfo(screen[0]);
  DrawUfo(screen[1]);
  if (ufo_pos < -37) {
    ring_area.y = 0;
    BitmapClearArea(screen[0], &ring_area);
    BitmapClearArea(screen[1], &ring_area);
    phase = END;
  }
}


static CopListT *MakeCopperList(void) {
  cp = NewCopList(128);
  bplptr = CopSetupBitplanes(cp, screen[active], DEPTH);

  sprptr = CopSetupSprites(cp);

  CopInsSetSprite(&sprptr[0], &coq);
  CopInsSetSprite(&sprptr[2], &mid_beam);
  CopInsSetSprite(&sprptr[4], &side_beam_l);
  CopInsSetSprite(&sprptr[5], &side_beam_r);

  SpriteUpdatePos(&coq,         X(152), Y(coq_pos));
  SpriteUpdatePos(&mid_beam,    X(152), Y(56));
  SpriteUpdatePos(&side_beam_l, X(137), Y(56));
  SpriteUpdatePos(&side_beam_r, X(167), Y(56));

  beam_pal_cp = CopLoadColors(cp, beam_pal[0], 21);

  return cp;
}

static void Init(void) {
  EnableDMA(DMAF_BLITTER | DMAF_RASTER | DMAF_SPRITE);
  // EnableDMA(DMAF_BLITHOG);

  screen[0] = NewBitmap(WIDTH, HEIGHT, DEPTH, BM_CLEAR);
  screen[1] = NewBitmap(WIDTH, HEIGHT, DEPTH, BM_CLEAR);
  SetupPlayfield(MODE_LORES, DEPTH, X(0), Y(0), WIDTH, HEIGHT);

  SetColor(0,  0x034);
  SetColor(1,  0x000);
  SetColor(2,  0xFFF);
  SetColor(3,  0x9EF);
  SetColor(4,  0x6DF);
  SetColor(5,  0x3BE);
  SetColor(6,  0x29B);
  SetColor(7,  0x289);
  SetColor(8,  0x157);
  SetColor(9,  0x035);
  SetColor(10, 0x034);

  // Unused
  SetColor(11, 0xF0F);
  SetColor(12, 0xF0F);
  SetColor(13, 0xF0F);
  SetColor(14, 0xF0F);
  SetColor(15, 0xF0F);

  SetColor(16, 0x000);
  // Coq
  SetColor(17, 0x035);
  SetColor(18, 0x2AD);
  SetColor(19, 0x00F);
  // Beam
  SetColor(21, 0xCEF);
  SetColor(22, 0xCEF);
  SetColor(23, 0xCEF);

  SetColor(25, 0x2AD);
  SetColor(26, 0x079);
  SetColor(27, 0x046);

  cp = MakeCopperList();
  CopListActivate(cp);

  custom->bplcon2 = BPLCON2_PF2PRI;

  DrawBackground(screen[0]);
  DrawBackground(screen[1]);
  DrawUfo(screen[0]);
  DrawUfo(screen[1]);
}

static void Kill(void) {
  DisableDMA(DMAF_BLITTER | DMAF_RASTER | DMAF_SPRITE);
  // DisableDMA(DMAF_BLITHOG);

  DeleteCopList(cp);
  DeleteBitmap(screen[0]);
  DeleteBitmap(screen[1]);
}

PROFILE(Abduction);

static void Render(void) {
  (void)ClearRing;
  (void)phase;
  ProfilerStart(Abduction);
  {
    ++counter;

    switch (phase) {
      case ABDUCT:
        Abduct();
        break;

      case RETRACT_BEAM:
        RetractBeam();
        break;

      case ESCAPE:
        Escape();
        break;

      case END:
        break;
    }
  }
  ProfilerStop(Abduction);

  TaskWaitVBlank();
  active ^= 1;
}

EFFECT(Abduction, NULL, NULL, Init, Kill, Render, NULL);