#include "effect.h"
#include "copper.h"
#include "gfx.h"
#include "blitter.h"
#include "circle.h"
#include "fx.h"
#include "2d.h"
#include "sprite.h"
#include <stdlib.h>

#define WIDTH 320
#define HEIGHT 256
#define DEPTH 4
#define HSIZE 32
#define VSIZE 10

#define LANE_W (WIDTH + HSIZE * 2)
#define LANE_H 40
#define CARS 50

#define LANEL_Y (HEIGHT / 2 - LANE_H - 16)
#define LANER_Y (HEIGHT / 2 + 16)

typedef struct {
  short speed;
  short x, y;
  bool active;
  bool side;
} Car;

static Car cars[CARS];

static CopListT *cp;
static u_short active = 0;
static CopInsT *sprptr[8];
static CopInsT *bplptr[2][DEPTH];
static BitmapT *carry;

#include "data/car-left-2.c"
#include "data/car-right-2.c"
#include "data/city-bottom-2.c"
#include "data/city-top-2.c"
#include "data/lane.c"
#include "data/sprite.c"

static BitmapT *lanes[2];

static void MakeCopperList(CopListT *cp) {
  CopInit(cp);
  CopSetupSprites(cp, sprptr);

  CopSetupBitplanes(cp, NULL, &city_top, DEPTH);
  CopWait(cp, Y(-18), 0);
  CopLoadPal(cp, &city_top_pal, 0);

  CopMove16(cp, dmacon, DMAF_SETCLR | DMAF_RASTER);

  {
    CopWait(cp, Y(LANEL_Y - 2), 8);
    CopMove16(cp, dmacon, DMAF_RASTER);
    CopLoadPal(cp, &car_left_pal, 0);
    CopSetupBitplanes(cp, bplptr[0], lanes[active], DEPTH);
    CopMove16(cp, bpl1mod, 8);
    CopMove16(cp, bpl2mod, 8);

    CopWait(cp, Y(LANEL_Y), 8);
    CopMove16(cp, dmacon, DMAF_SETCLR | DMAF_RASTER);

    CopWait(cp, Y(LANEL_Y + LANE_H), 8);
    CopMove16(cp, dmacon, DMAF_RASTER);
  }

  // use an undocumented trick to make sprites visible while bitplanes are off
  {
    short y0 = LANEL_Y + LANE_H + 1;
    short y1 = LANER_Y - 2;

    while (y0 < y1) {
      CopWait(cp, Y(y0), X(-12));
      CopMove16(cp, bpldat[0], 0);
      y0++;
    }
  }

  {
    CopWait(cp, Y(LANER_Y - 1), 8);
    CopLoadPal(cp, &car_right_pal, 0);
    CopSetupBitplanes(cp, bplptr[1], lanes[active], DEPTH);
    CopMove16(cp, bpl1mod, 8);
    CopMove16(cp, bpl2mod, 8);

    CopWait(cp, Y(LANER_Y), 8);
    CopMove16(cp, dmacon, DMAF_SETCLR | DMAF_RASTER);
    CopWait(cp, Y(LANER_Y + LANE_H), 8);
    CopMove16(cp, dmacon, DMAF_RASTER);
  }

  {
    CopWait(cp, Y(LANER_Y + LANE_H + 1), 8);
    CopLoadPal(cp, &city_bottom_pal, 0);
    CopSetupBitplanes(cp, NULL, &city_bottom, DEPTH);
    CopWait(cp, Y(LANER_Y + LANE_H + 2), 8);
    CopMove16(cp, dmacon, DMAF_SETCLR | DMAF_RASTER);
  }

  CopEnd(cp);

  ITER(i, 0, 7, CopInsSetSprite(sprptr[i], sprite[i]));
}

static void Init(void) {
  lanes[0] = NewBitmap(LANE_W, LANE_H * 2, DEPTH);
  lanes[1] = NewBitmap(LANE_W, LANE_H * 2, DEPTH);
  carry = NewBitmap(HSIZE + 16, VSIZE, 2);

  SetupPlayfield(MODE_LORES, DEPTH, X(0), Y(0), WIDTH, HEIGHT);

  LoadPalette(&sprite_pal, 16);
  LoadPalette(&sprite_pal, 20);
  LoadPalette(&sprite_pal, 24);
  LoadPalette(&sprite_pal, 28);

  cp = NewCopList(300);
  MakeCopperList(cp);
  CopListActivate(cp);
  EnableDMA(DMAF_RASTER | DMAF_BLITTER | DMAF_SPRITE);

  ITER(i, 0, 7,
       SpriteUpdatePos(sprite[i], X(96 + 16 * i), Y(LANEL_Y + LANE_H + 4)));
}

static void Kill(void) {
  DisableDMA(DMAF_COPPER | DMAF_RASTER | DMAF_BLITTER | DMAF_SPRITE);

  DeleteBitmap(lanes[0]);
  DeleteBitmap(lanes[1]);
  DeleteBitmap(carry);
  DeleteCopList(cp);
}

static inline void CarInit(Car *car) {
  car->speed = random() & 15;
  car->x = 0;
  car->y = (random() & 3) * 10;
  car->active = true;
  car->side = random() & 1;
}

static inline void CarMove(Car *car, short step) {
  car->x += (car->speed + 32) * step;
  if (car->x >= fx4i(LANE_W))
    car->active = false;
}

/* Add new car if there's a free slot. */
static inline void AddCar(void) {
  Car *car = cars;
  Car *last = cars + CARS;

  do {
    if (!car->active) {
      CarInit(car);
      break;
    }
  } while (++car < last);
}

/* Draw each active cars. */
static void DrawCars(short step) {
  Car *car = cars;
  Car *last = cars + CARS;

  do {
    if (car->active) {
      short x = (car->x + 7) / 16;

      if (car->side)
        BitmapAddSaturated(lanes[active], x, car->y + LANE_H, &car_right, carry);
      else
        BitmapAddSaturated(lanes[active], LANE_W - x, car->y, &car_left, carry);

      CarMove(car, step);
    }
  } while (++car < last);
}

static void AddCars(void) {
  static short iterCount = 0;

  iterCount += frameCount - lastFrameCount;
  while (iterCount > 10) {
    AddCar();
    iterCount -= 10;
  }
}

static void Render(void) {
  BitmapCopy(lanes[active], HSIZE, 0, &lane_bg);
  BitmapCopy(lanes[active], HSIZE, LANE_H, &lane_bg);

  AddCars();
  DrawCars(frameCount - lastFrameCount);

  {
    short i;

    for (i = 0; i < DEPTH; i++) {
      void *bplpt = lanes[active]->planes[i] + 4;
      u_short stride = lanes[active]->bytesPerRow;
      CopInsSet32(bplptr[0][i], bplpt);
      CopInsSet32(bplptr[1][i], bplpt + stride * LANE_H);
    }
  }

  TaskWaitVBlank();
  active ^= 1;
}

EFFECT(highway, NULL, NULL, Init, Kill, Render);
