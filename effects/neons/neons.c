#include "effect.h"
#include "interrupt.h"
#include "copper.h"
#include "gfx.h"
#include "blitter.h"
#include "2d.h"
#include "fx.h"
#include "random.h"

#define WIDTH 320
#define HEIGHT 256
#define DEPTH 5

#define PNUM 19

static BitmapT *screen[2];
static u_short active = 0;
static CopInsT *bplptr[5];

static const PaletteT *palette[3];
static CopListT *cp;
static CopInsT *pal;

#include "data/greet_ada.c"
#include "data/greet_blacksun.c"
#include "data/greet_dcs.c"
#include "data/greet_dekadence.c"
#include "data/greet_desire.c"
#include "data/greet_dinx.c"
#include "data/greet_elude.c"
#include "data/greet_fd.c"
#include "data/greet_floppy.c"
#include "data/greet_lemon.c"
#include "data/greet_loonies.c"
#include "data/greet_moods.c"
#include "data/greet_nah.c"
#include "data/greet_rno.c"
#include "data/greet_skarla.c"
#include "data/greet_speccy.c"
#include "data/greet_tulou.c"
#include "data/greet_wanted.c"
#include "data/greet_ycrew.c"
#include "data/neons.c"

static Area2D grt_area[2][PNUM];

typedef struct {
  short color;
  const BitmapT *bitmap;
  Point2D pos;
} GreetingT;

#define GREETING(color, group) {(color), &(greet_ ## group), {0, 0}}

static GreetingT greeting[PNUM] = {
  GREETING(0, ada),
  GREETING(0, blacksun),
  GREETING(1, dcs),
  GREETING(0, dekadence),
  GREETING(1, desire),
  GREETING(0, dinx),
  GREETING(1, elude),
  GREETING(0, fd),
  GREETING(1, floppy),
  GREETING(0, lemon),
  GREETING(1, loonies),
  GREETING(1, moods),
  GREETING(0, nah),
  GREETING(0, rno),
  GREETING(1, skarla),
  GREETING(0, speccy),
  GREETING(0, tulou),
  GREETING(1, wanted),
  GREETING(1, ycrew)
};

static void PositionGreetings(void) {
  GreetingT *grt = greeting;
  short y = HEIGHT + 200;
  short i;
  
  for (i = 0; i < PNUM; i++) {
    Point2D *pos = &grt->pos;
    const BitmapT *src = grt->bitmap;

    pos->x = (i & 1) ? (WIDTH / 2 - 64) : (WIDTH / 2 + 64 - src->width);
    pos->y = y;

    y += src->height / 2 + (random() & 31) + 10;

    grt++;
  }
}

static void Load(void) {
  screen[0] = NewBitmap(WIDTH, HEIGHT, DEPTH);
  screen[1] = NewBitmap(WIDTH, HEIGHT, DEPTH);

  palette[0] = &background_pal;
  palette[1] = &moods_pal;
  palette[2] = &rno_pal;

  PositionGreetings();
}

static void UnLoad(void) {
  DeleteBitmap(screen[0]);
  DeleteBitmap(screen[1]);
}

static __interrupt int CustomRotatePalette(void) {
  u_short *src = palette[0]->colors;
  CopInsT *ins = pal + 1;
  int i = frameCount;
  short n = 15;

  while (--n >= 0)
    CopInsSet16(ins++, src[i++ & 15]);

  return 0;
}

INTERRUPT(RotatePaletteInterrupt, 0, CustomRotatePalette, NULL);

static void Init(void) {
  EnableDMA(DMAF_BLITTER);

  BitmapCopy(screen[0], 0, 0, &background);
  BitmapCopy(screen[1], 0, 0, &background);

  BlitterClear(screen[0], 4);
  BlitterClear(screen[1], 4);

  cp = NewCopList(100);

  CopInit(cp);
  CopSetupGfxSimple(cp, MODE_LORES, DEPTH, X(0), Y(0), WIDTH, HEIGHT);
  CopSetupBitplanes(cp, bplptr, screen[active], DEPTH);
  pal = CopLoadPal(cp, palette[0], 0);
  CopLoadPal(cp, palette[1], 16);
  CopLoadPal(cp, palette[2], 24);
  CopEnd(cp);

  CopListActivate(cp);
  EnableDMA(DMAF_RASTER);

  AddIntServer(INTB_VERTB, RotatePaletteInterrupt);
}

static void Kill(void) {
  DisableDMA(DMAF_COPPER | DMAF_RASTER | DMAF_BLITTER);

  RemIntServer(INTB_VERTB, RotatePaletteInterrupt);

  DeleteCopList(cp);
}

static void ClearCliparts(void) {
  Area2D *area = grt_area[active];
  BitmapT *dst = screen[active];
  short n = PNUM;

  while (--n >= 0) {
    Area2D neon = *area++;

    if (neon.h > 0) {
      if (neon.h > 8) {
        neon.y += neon.h - 8;
        neon.h = 8; 
      }
      BlitterClearArea(dst, 4, &neon);
      BitmapCopyArea(dst, neon.x, neon.y, &background, &neon);
    }
  }
}

static void DrawCliparts(void) {
  GreetingT *grt = greeting;
  Area2D *area = grt_area[active];
  BitmapT *dst = screen[active];
  short step = (frameCount - lastFrameCount) * 3;
  short n = PNUM;

  while (--n >= 0) {
    const BitmapT *src = grt->bitmap;
    short dy = grt->pos.y;
    short sy = 0;
    short sh = src->height;

    if (dy < 0) { sy -= dy; sh += dy; dy = 0; }
    if (dy + sh >= HEIGHT) { sh = HEIGHT - dy; }

    if (sh > 0) {
      Area2D bg_area = { grt->pos.x, dy, src->width, sh };
      Area2D fg_area = { 0, sy, src->width, sh };

      area->x = grt->pos.x;
      area->y = dy;
      area->w = src->width;
      area->h = sh;

      BitmapCopyArea(dst, grt->pos.x, dy, src, &fg_area);
      BlitterSetArea(dst, 3, &bg_area, grt->color ? 0 : -1);
      BlitterSetArea(dst, 4, &bg_area, -1);
    } else {
      area->h = 0;
    }

    grt->pos.y -= step;
    grt++;
    area++;
  }
}

PROFILE(RenderNeons);

static void Render(void) {
  ProfilerStart(RenderNeons);
  {
    WaitBlitter();
    ClearCliparts();
    DrawCliparts();
  }
  ProfilerStop(RenderNeons);

  ITER(i, 0, DEPTH - 1, CopInsSet32(bplptr[i], screen[active]->planes[i]));
  TaskWaitVBlank();
  active ^= 1;
}

EFFECT(neons, Load, UnLoad, Init, Kill, Render);
