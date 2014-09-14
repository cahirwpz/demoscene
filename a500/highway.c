#include "startup.h"
#include "hardware.h"
#include "coplist.h"
#include "gfx.h"
#include "ilbm.h"
#include "blitter.h"
#include "circle.h"
#include "fx.h"
#include "2d.h"
#include "random.h"
#include "sprite.h"

#define WIDTH 320
#define HEIGHT 256
#define HSIZE 32
#define VSIZE 10

#define LANE_W (WIDTH + HSIZE * 2)
#define LANE_H 40
#define CARS 50

#define LANEL_Y (HEIGHT / 2 - LANE_H - 16)
#define LANER_Y (HEIGHT / 2 + 16)

typedef struct {
  WORD speed;
  WORD x, y;
  BOOL active;
  BOOL side;
} Car;

static Car cars[CARS];

static CopListT *cp[2];
static UWORD active = 0;
static CopInsT *sprptr[2][8];

static BitmapT *carry;
static BitmapT *lanes[2];
static BitmapT *carsL;
static BitmapT *carsR;
static BitmapT *carsBg;
static BitmapT *carsTop;
static BitmapT *carsBottom;
static SpriteT *nullspr;
static BitmapT *title;
static SpriteT *sprite[8];

static void MakeCopperList(CopListT *cp, WORD num) {
  CopInit(cp);
  CopMakeSprites(cp, sprptr[num], nullspr);
  CopLoadPal(cp, title->palette, 16);
  CopLoadPal(cp, title->palette, 20);
  CopLoadPal(cp, title->palette, 24);
  CopLoadPal(cp, title->palette, 28);

  CopMakeDispWin(cp, X(0), Y(0), WIDTH, HEIGHT);
  CopShowPlayfield(cp, carsTop);
  CopLoadPal(cp, carsTop->palette, 0);

  CopMove16(cp, dmacon, DMAF_SETCLR | DMAF_RASTER);

  {
    CopWait(cp, Y(LANEL_Y - 1), 8);
    CopMove16(cp, dmacon, DMAF_RASTER);
    CopLoadPal(cp, carsL->palette, 0);
    CopShowPlayfieldArea(cp, lanes[num], 32, 0, WIDTH);

    CopWait(cp, Y(LANEL_Y), 8);
    CopMove16(cp, dmacon, DMAF_SETCLR | DMAF_RASTER);

    CopWait(cp, Y(LANEL_Y + LANE_H), 8);
    CopSetRGB(cp, 0, 0x0222);
    CopMove16(cp, dmacon, DMAF_RASTER);
  }

  {
    WORD y0 = LANEL_Y + LANE_H + 1;
    WORD y1 = LANER_Y - 2;

    while (y0 < y1) {
      CopWait(cp, Y(y0), 8);
      CopMove16(cp, bpldat[0], 0);
      y0++;
    }
  }

  {
    CopWait(cp, Y(LANER_Y - 1), 8);
    CopLoadPal(cp, carsR->palette, 0);
    CopShowPlayfieldArea(cp, lanes[num], 32, LANE_H, WIDTH);

    CopWait(cp, Y(LANER_Y), 8);
    CopMove16(cp, dmacon, DMAF_SETCLR | DMAF_RASTER);
    CopWait(cp, Y(LANER_Y + LANE_H), 8);
    CopMove16(cp, dmacon, DMAF_RASTER);
  }

  {
    CopWait(cp, Y(LANER_Y + LANE_H + 1), 8);
    CopLoadPal(cp, carsBottom->palette, 0);
    CopShowPlayfield(cp, carsBottom);
    CopWait(cp, Y(LANER_Y + LANE_H + 2), 8);
    CopMove16(cp, dmacon, DMAF_SETCLR | DMAF_RASTER);
  }

  CopEnd(cp);

  ITER(i, 0, 7, CopInsSet32(sprptr[num][i], sprite[i]->data));
}

static void Load() {
  lanes[0] = NewBitmap(LANE_W, LANE_H * 2, 4, FALSE);
  lanes[1] = NewBitmap(LANE_W, LANE_H * 2, 4, FALSE);
  title = LoadILBM("data/sprite128.ilbm", FALSE);
  carsBg = LoadILBM("data/cars-bg.ilbm", FALSE);
  carsTop = LoadILBM("data/cars-top.ilbm", FALSE);
  carsBottom = LoadILBM("data/cars-bottom.ilbm", FALSE);
  carsL = LoadILBM("data/cars-l.ilbm", FALSE);
  carsR = LoadILBM("data/cars-r.ilbm", FALSE);
  carry = NewBitmap(HSIZE + 16, VSIZE, 2, FALSE);
  nullspr = NewSprite(0, FALSE);

  ITER(i, 0, 7, sprite[i] = NewSpriteFromBitmap(24, title, 16 * i, 0));
  ITER(i, 0, 7, UpdateSpritePos(sprite[i], X(80 + 16 * i), Y(LANEL_Y + LANE_H + 4)));

  cp[0] = NewCopList(300);
  cp[1] = NewCopList(300);
  MakeCopperList(cp[0], 0);
  MakeCopperList(cp[1], 1);
}

static void UnLoad() {
  DeleteSprite(nullspr);
  ITER(i, 0, 7, DeleteSprite(sprite[i]));
  DeletePalette(carsL->palette);
  DeleteBitmap(carsL);
  DeletePalette(carsR->palette);
  DeleteBitmap(carsR);
  DeletePalette(carsTop->palette);
  DeleteBitmap(carsTop);
  DeletePalette(carsBottom->palette);
  DeleteBitmap(carsBottom);
  DeletePalette(carsBg->palette);
  DeleteBitmap(carsBg);
  DeleteBitmap(carry);
  DeleteBitmap(lanes[0]);
  DeleteBitmap(lanes[1]);
  DeleteBitmap(title);
  DeleteCopList(cp[0]);
  DeleteCopList(cp[1]);
}

static inline void CarInit(Car *car) {
  car->speed = random() & 15;
  car->x = 0;
  car->y = (random() & 3) * 10;
  car->active = TRUE;
  car->side = random() & 1;
}

static inline void CarMove(Car *car, WORD step) {
  car->x += (car->speed + 32) * step;
  if (car->x >= fx4i(LANE_W))
    car->active = FALSE;
}

/* Add new car if there's a free slot. */
static void AddCar() {
  WORD i;

  for (i = 0; i < CARS; i++) {
    Car *car = &cars[i];

    if (!car->active) {
      CarInit(car);
      break;
    }
  }
}

/* Bitplane adder with saturation. */
static void BlitterAddSaturatedSync(BitmapT *dst, WORD dx, WORD dy, BitmapT *src) {
  ULONG dst_begin = ((dx & ~15) >> 3) + dy * dst->bytesPerRow;
  UWORD dst_modulo = (dst->bytesPerRow - src->bytesPerRow) - 2;
  UWORD src_shift = (dx & 15) << ASHIFTSHIFT;
  UWORD bltsize = (src->height << 6) | ((src->width + 16) >> 4);
  APTR *__src = src->planes;
  APTR *__dst = dst->planes;
  APTR *__carry = carry->planes;
  WORD i, k;

  WaitBlitter();

  /* Initialize blitter */
  custom->bltamod = -2;
  custom->bltbmod = dst_modulo;
  custom->bltcmod = 0;
  custom->bltcon1 = 0;
  custom->bltafwm = -1;
  custom->bltalwm = 0;

  /* Bitplane 0: half adder with carry. */
  custom->bltapt = __src[0];
  custom->bltbpt = __dst[0] + dst_begin;
  custom->bltdpt = __carry[0];
  custom->bltdmod = 0;
  custom->bltcon0 = HALF_ADDER_CARRY | src_shift;
  custom->bltsize = bltsize;

  WaitBlitter();
  custom->bltapt = __src[0];
  custom->bltbpt = __dst[0] + dst_begin;
  custom->bltdpt = __dst[0] + dst_begin;
  custom->bltdmod = dst_modulo;
  custom->bltcon0 = HALF_ADDER | src_shift;
  custom->bltsize = bltsize;

  /* Bitplane 1-n: full adder with carry. */
  for (i = 1, k = 0; i < dst->depth; i++, k ^= 1) {
    WaitBlitter();
    custom->bltapt = __src[i];
    custom->bltbpt = __dst[i] + dst_begin;
    custom->bltcpt = __carry[k];
    custom->bltdpt = __carry[k ^ 1];
    custom->bltdmod = 0;
    custom->bltcon0 = FULL_ADDER_CARRY | src_shift;
    custom->bltsize = bltsize;

    WaitBlitter();
    custom->bltapt = __src[i];
    custom->bltbpt = __dst[i] + dst_begin;
    custom->bltcpt = __carry[k];
    custom->bltdpt = __dst[i] + dst_begin;
    custom->bltdmod = dst_modulo;
    custom->bltcon0 = FULL_ADDER | src_shift;
    custom->bltsize = bltsize;
  }

  /* Apply saturation bits. */
  WaitBlitter();
  custom->bltamod = dst_modulo;
  custom->bltbmod = 0;
  custom->bltdmod = dst_modulo;
  custom->bltcon0 = (SRCA | SRCB | DEST) | A_OR_B;
  custom->bltalwm = -1;

  for (i = 0; i < dst->depth; i++) {
    WaitBlitter();
    custom->bltapt = __dst[i] + dst_begin;
    custom->bltbpt = __carry[k];
    custom->bltdpt = __dst[i] + dst_begin;
    custom->bltsize = bltsize;
  }
}

/* Draw each active cars. */
static void DrawCars(WORD step) {
  WORD i;

  for (i = 0; i < CARS; i++) {
    Car *car = &cars[i];

    if (car->active) {
      WORD x = (car->x + 7) / 16;

      if (car->side)
        BlitterAddSaturatedSync(lanes[active], x, car->y + LANE_H, carsR);
      else
        BlitterAddSaturatedSync(lanes[active], LANE_W - x, car->y, carsL);

      CarMove(car, step);
    }
  }
}

static WORD iterCount = 0;
static LONG lastFrameCount;
static LONG frameCount = 0;

static void Init() {
  CopListActivate(cp[active]);
  custom->dmacon = DMAF_SETCLR | DMAF_RASTER | DMAF_BLITTER | DMAF_SPRITE;

  lastFrameCount = ReadFrameCounter();
}

static void Render() {
  frameCount = ReadFrameCounter();

#if 0
  ITER(i, 0, 3, BlitterSetSync(laneL[active], i, HSIZE, 0, WIDTH, LANE_H, 0));
  ITER(i, 0, 3, BlitterSetSync(laneR[active], i, HSIZE, LANE_H, WIDTH, LANE_H, 0));
#else
  ITER(i, 0, 3, BlitterCopySync(lanes[active], i, HSIZE, 0, carsBg, i));
  ITER(i, 0, 3, BlitterCopySync(lanes[active], i, HSIZE, LANE_H, carsBg, i));
#endif

  iterCount += frameCount - lastFrameCount;
  while (iterCount > 10) {
    AddCar();
    iterCount -= 10;
  }

  DrawCars(frameCount - lastFrameCount);
  lastFrameCount = frameCount;

  CopListRun(cp[active]);
  WaitVBlank();
  active ^= 1;
}

EffectT Effect = { Load, UnLoad, Init, NULL, Render };
