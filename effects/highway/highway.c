#include <effect.h>
#include <copper.h>
#include <blitter.h>
#include <fx.h>
#include <sprite.h>
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
static short active = 0;
static CopInsPairT *bplptr[2];
static BitmapT *carry;

#include "data/car-left-2.c"
#include "data/car-right-2.c"
#include "data/city-bottom-2.c"
#include "data/city-top-2.c"
#include "data/lane.c"
#include "data/sprite.c"

static BitmapT *lanes[2];

static CopListT *MakeCopperList(void) {
  CopListT *cp = NewCopList(300);
  CopInsPairT *sprptr = CopSetupSprites(cp);
  short i;

  CopSetupBitplanes(cp, &city_top, DEPTH);
  CopWait(cp, Y(-18), HP(0));
  CopLoadColors(cp, city_top_colors, 0);

  CopMove16(cp, dmacon, DMAF_SETCLR | DMAF_RASTER);

  {
    CopWait(cp, Y(LANEL_Y - 2), HP(16));
    CopMove16(cp, dmacon, DMAF_RASTER);
    CopLoadColors(cp, car_left_colors, 0);
    bplptr[0] = CopSetupBitplanes(cp, lanes[active], DEPTH);
    CopMove16(cp, bpl1mod, 8);
    CopMove16(cp, bpl2mod, 8);

    CopWait(cp, Y(LANEL_Y), HP(16));
    CopMove16(cp, dmacon, DMAF_SETCLR | DMAF_RASTER);

    CopWait(cp, Y(LANEL_Y + LANE_H), HP(16));
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
    CopWait(cp, Y(LANER_Y - 1), HP(16));
    CopLoadColors(cp, car_right_colors, 0);
    bplptr[1] = CopSetupBitplanes(cp, lanes[active], DEPTH);
    CopMove16(cp, bpl1mod, 8);
    CopMove16(cp, bpl2mod, 8);

    CopWait(cp, Y(LANER_Y), HP(16));
    CopMove16(cp, dmacon, DMAF_SETCLR | DMAF_RASTER);
    CopWait(cp, Y(LANER_Y + LANE_H), HP(16));
    CopMove16(cp, dmacon, DMAF_RASTER);
  }

  {
    CopWait(cp, Y(LANER_Y + LANE_H + 1), HP(16));
    CopLoadColors(cp, city_bottom_colors, 0);
    CopSetupBitplanes(cp, &city_bottom, DEPTH);
    CopWait(cp, Y(LANER_Y + LANE_H + 2), HP(16));
    CopMove16(cp, dmacon, DMAF_SETCLR | DMAF_RASTER);
  }

  for (i = 0; i < 8; i++) {
    CopInsSetSprite(&sprptr[i], sprite[i]);
    SpriteUpdatePos(sprite[i], X(96 + 16 * i), Y(LANEL_Y + LANE_H + 4));
  }

  return CopListFinish(cp);
}

static void Init(void) {
  lanes[0] = NewBitmap(LANE_W, LANE_H * 2, DEPTH, BM_CLEAR);
  lanes[1] = NewBitmap(LANE_W, LANE_H * 2, DEPTH, BM_CLEAR);
  carry = NewBitmap(HSIZE + 16, VSIZE, 2, 0);

  SetupPlayfield(MODE_LORES, DEPTH, X(0), Y(0), WIDTH, HEIGHT);

  LoadColors(sprite_colors, 16);
  LoadColors(sprite_colors, 20);
  LoadColors(sprite_colors, 24);
  LoadColors(sprite_colors, 28);

  cp = MakeCopperList();
  CopListActivate(cp);
  EnableDMA(DMAF_RASTER | DMAF_BLITTER | DMAF_SPRITE);
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
      CopInsSet32(&bplptr[0][i], bplpt);
      CopInsSet32(&bplptr[1][i], bplpt + stride * LANE_H);
    }
  }

  TaskWaitVBlank();
  active ^= 1;
}

EFFECT(HighWay, NULL, NULL, Init, Kill, Render, NULL);
