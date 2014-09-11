#include "hardware.h"
#include "coplist.h"
#include "gfx.h"
#include "ilbm.h"
#include "interrupts.h"
#include "blitter.h"
#include "circle.h"
#include "fx.h"
#include "2d.h"
#include "random.h"
#include "sprite.h"

#define X(x) ((x) + 0x81)
#define Y(y) ((y) + 0x2c)

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

static UWORD active = 0;
static CopInsT *bplptr[2][5];

static BitmapT *carry;
static BitmapT *lanes[2];
static BitmapT *carsL;
static BitmapT *carsR;
static BitmapT *carsBg;
static BitmapT *carsTop;
static BitmapT *carsBottom;
static SpriteT *nullspr;
static BitmapT *sprite128;
static CopInsT *sprptr[8];
static SpriteT *sprite[8];

static CopListT *cp;

void Load() {
  lanes[0] = NewBitmap(LANE_W, LANE_H * 2, 4, FALSE);
  lanes[1] = NewBitmap(LANE_W, LANE_H * 2, 4, FALSE);
  sprite128 = LoadILBM("data/sprite128.ilbm", FALSE);
  carsBg = LoadILBM("data/cars-bg.ilbm", FALSE);
  carsTop = LoadILBM("data/cars-top.ilbm", FALSE);
  carsBottom = LoadILBM("data/cars-bottom.ilbm", FALSE);
  carsL = LoadILBM("data/cars-l.ilbm", FALSE);
  carsR = LoadILBM("data/cars-r.ilbm", FALSE);
  carry = NewBitmap(HSIZE + 16, VSIZE, 2, FALSE);

  nullspr = NewSprite(0, FALSE);

  cp = NewCopList(500);
  CopInit(cp);
  CopMakeDispWin(cp, X(0), Y(0), WIDTH, HEIGHT);
  CopMakePlayfield(cp, NULL, carsTop);
  CopLoadPal(cp, carsTop->palette, 0);

  CopMakeSprites(cp, sprptr, nullspr);
  CopLoadPal(cp, sprite128->palette, 16);
  CopLoadPal(cp, sprite128->palette, 20);
  CopLoadPal(cp, sprite128->palette, 24);
  CopLoadPal(cp, sprite128->palette, 28);

  CopMove16(cp, dmacon, DMAF_SETCLR | DMAF_RASTER);

  {
    CopWait(cp, Y(64), 8);
    CopMove16(cp, dmacon, DMAF_RASTER);
    CopSetRGB(cp, 0, 0x0222);

    CopWait(cp, Y(LANEL_Y - 1), 8);
    CopLoadPal(cp, carsL->palette, 0);
    CopMakePlayfield(cp, bplptr[0], lanes[active]);
    CopMove16(cp, bpl1mod, 2 * HSIZE / 8);
    CopMove16(cp, bpl2mod, 2 * HSIZE / 8);

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
    CopMakePlayfield(cp, bplptr[1], lanes[active]);
    CopMove16(cp, bpl1mod, 2 * HSIZE / 8);
    CopMove16(cp, bpl2mod, 2 * HSIZE / 8);

    CopWait(cp, Y(LANER_Y), 8);
    CopMove16(cp, dmacon, DMAF_SETCLR | DMAF_RASTER);
    CopWait(cp, Y(LANER_Y + LANE_H), 8);
    CopMove16(cp, dmacon, DMAF_RASTER);

    CopWait(cp, Y(LANER_Y + LANE_H + 1), 8);
    CopSetRGB(cp, 0, 0x222);
  }

  {
    CopWait(cp, Y(192 - 1), 8);
    CopSetRGB(cp, 0, 0);
    CopMakePlayfield(cp, NULL, carsBottom);
    CopLoadPal(cp, carsBottom->palette, 0);
    CopWait(cp, Y(192), 8);
    CopMove16(cp, dmacon, DMAF_SETCLR | DMAF_RASTER);
  }

  CopEnd(cp);

  ITER(i, 0, 7, sprite[i] = NewSpriteFromBitmap(24, sprite128, 16 * i, 0));
  ITER(i, 0, 7, UpdateSpritePos(sprite[i], X(80 + 16 * i), Y(LANEL_Y + LANE_H + 4)));
  ITER(i, 0, 7, CopInsSet32(sprptr[i], sprite[i]->data));
}

void Kill() {
  DeleteSprite(nullspr);
  ITER(i, 0, 7, DeleteSprite(sprite[i]));
  DeleteCopList(cp);
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
  DeleteBitmap(sprite128);
}

static volatile LONG swapScreen = -1;
static volatile LONG frameCount = 0;

__interrupt_handler void IntLevel3Handler() {
  if (custom->intreqr & INTF_VERTB) {
    if (swapScreen >= 0) {
      BitmapT *buffer = lanes[swapScreen];
      WORD n = 4;

      while (--n >= 0) {
        CopInsSet32(bplptr[0][n], buffer->planes[n] + HSIZE / 8);
        CopInsSet32(bplptr[1][n], buffer->planes[n] + HSIZE / 8 +
                    buffer->bytesPerRow * LANE_H);
      }

      swapScreen = -1;
    }

    ITER(i, 0, 7, UpdateSprite(sprite[i]));

    frameCount++;
  }

  custom->intreq = INTF_LEVEL3;
  custom->intreq = INTF_LEVEL3;
}

static inline void CarInit(Car *car) {
  car->speed = random() & 15;
  car->x = 0;
  car->y = (random() & 3) * 10;
  car->active = TRUE;
  car->side = random() & 1;
}

static inline void CarMove(Car *car, WORD step) {
  car->x += (car->speed + 8) * step;
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
void BlitterAddSaturatedSync(BitmapT *dst, WORD dx, WORD dy, BitmapT *src) {
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

static BOOL Loop() {
  // LONG lines = ReadLineCounter();

#if 0
  ITER(i, 0, 3, BlitterSetSync(laneL[active], i, HSIZE, 0, WIDTH, LANE_H, 0));
  ITER(i, 0, 3, BlitterSetSync(laneR[active], i, HSIZE, LANE_H, WIDTH, LANE_H, 0));
#else
  ITER(i, 0, 3, BlitterCopySync(lanes[active], i, HSIZE, 0, carsBg, i));
  ITER(i, 0, 3, BlitterCopySync(lanes[active], i, HSIZE, LANE_H, carsBg, i));
#endif

  if ((iterCount++ & 7) == 0)
    AddCar();

  {
    LONG t = frameCount;
    DrawCars(frameCount - lastFrameCount);
    lastFrameCount = t;
  }

  // Log("loop: %ld\n", ReadLineCounter() - lines);

  WaitVBlank();

  swapScreen = active;
  active ^= 1;

  return !LeftMouseButton();
}

void Main() {
  InterruptVector->IntLevel3 = IntLevel3Handler;
  custom->intena = INTF_SETCLR | INTF_VERTB;
  
  CopListActivate(cp);
  custom->dmacon = DMAF_SETCLR | DMAF_RASTER | DMAF_BLITTER | DMAF_SPRITE;

  lastFrameCount = frameCount;

  while (Loop());
}
