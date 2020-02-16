#include "startup.h"
#include "hardware.h"
#include "interrupts.h"
#include "coplist.h"
#include "gfx.h"
#include "ilbm.h"
#include "blitter.h"
#include "2d.h"
#include "fx.h"
#include "random.h"
#include "tasks.h"

const char *__cwdpath = "data";

#define WIDTH 320
#define HEIGHT 256
#define DEPTH 5

#define PNUM 19

static BitmapT *screen[2];
static u_short active = 0;
static CopInsT *bplptr[5];

static BitmapT *background;
static PaletteT *palette[3];
static CopListT *cp;
static CopInsT *pal;

static Area2D grt_area[2][PNUM];

typedef struct {
  short color;
  const char *filename;
  BitmapT *bitmap;
  Point2D pos;
} GreetingT;

#define GREETING(color, filename) {(color), (filename), NULL, {0, 0}}

static GreetingT greeting[PNUM] = {
  GREETING(0, "greet_ada.ilbm"),
  GREETING(0, "greet_blacksun.ilbm"),
  GREETING(1, "greet_dcs.ilbm"),
  GREETING(0, "greet_dekadence.ilbm"),
  GREETING(1, "greet_desire.ilbm"),
  GREETING(0, "greet_dinx.ilbm"),
  GREETING(1, "greet_elude.ilbm"),
  GREETING(0, "greet_fd.ilbm"),
  GREETING(1, "greet_floppy.ilbm"),
  GREETING(0, "greet_lemon.ilbm"),
  GREETING(1, "greet_loonies.ilbm"),
  GREETING(1, "greet_moods.ilbm"),
  GREETING(0, "greet_nah.ilbm"),
  GREETING(0, "greet_rno.ilbm"),
  GREETING(1, "greet_skarla.ilbm"),
  GREETING(0, "greet_speccy.ilbm"),
  GREETING(0, "greet_tulou.ilbm"),
  GREETING(1, "greet_wanted.ilbm"),
  GREETING(1, "greet_ycrew.ilbm")
};

static void PositionGreetings(void) {
  GreetingT *grt = greeting;
  short y = HEIGHT + 200;
  short i;
  
  for (i = 0; i < PNUM; i++) {
    Point2D *pos = &grt->pos;
    BitmapT *src = grt->bitmap;

    pos->x = (i & 1) ? (WIDTH / 2 - 64) : (WIDTH / 2 + 64 - src->width);
    pos->y = y;

    y += src->height / 2 + (random() & 31) + 10;

    grt++;
  }
}

static void Load(void) {
  short i;

  screen[0] = NewBitmap(WIDTH, HEIGHT, DEPTH);
  screen[1] = NewBitmap(WIDTH, HEIGHT, DEPTH);

  for (i = 0; i < PNUM; i++)
    greeting[i].bitmap = LoadILBMCustom(greeting[i].filename, BM_DISPLAYABLE);

  background = LoadILBMCustom("neons.ilbm", BM_DISPLAYABLE|BM_LOAD_PALETTE);
  palette[0] = background->palette;
  palette[1] = LoadPalette("greet_moods.ilbm");
  palette[2] = LoadPalette("greet_rno.ilbm");

  PositionGreetings();
}

static void UnLoad(void) {
  ITER(i, 0, PNUM - 1, DeleteBitmap(greeting[i].bitmap));
  DeleteBitmap(background);
  DeletePalette(palette[0]);
  DeletePalette(palette[1]);
  DeletePalette(palette[2]);
  DeleteBitmap(screen[0]);
  DeleteBitmap(screen[1]);
}

static __interrupt int CustomRotatePalette(void) {
  ColorT *src = palette[0]->colors;
  CopInsT *ins = pal + 1;
  int i = frameCount;
  short n = 15;

  while (--n >= 0) {
    u_char *c = (u_char *)&src[i++ & 15];
    u_char r = *c++ & 0xf0;
    u_char g = *c++ & 0xf0;
    u_char b = *c++ & 0xf0;
    CopInsSet16(ins++, (r << 4) | (u_char)(g | (b >> 4)));
  }

  return 0;
}

INTERRUPT(RotatePaletteInterrupt, 0, CustomRotatePalette, NULL);

static void Init(void) {
  EnableDMA(DMAF_BLITTER);

  BitmapCopy(screen[0], 0, 0, background);
  BitmapCopy(screen[1], 0, 0, background);

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
      BitmapCopyArea(dst, neon.x, neon.y, background, &neon);
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
    BitmapT *src = grt->bitmap;
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

static void Render(void) {
  // int lines = ReadLineCounter();

  WaitBlitter();
  ClearCliparts();
  DrawCliparts();
  
  // Log("neons: %d\n", ReadLineCounter() - lines);

  ITER(i, 0, DEPTH - 1, CopInsSet32(bplptr[i], screen[active]->planes[i]));
  TaskWait(VBlankEvent);
  active ^= 1;
}

EffectT Effect = { Load, UnLoad, Init, Kill, Render, NULL };
