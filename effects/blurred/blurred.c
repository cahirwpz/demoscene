#include <effect.h>
#include <2d.h>
#include <blitter.h>
#include <circle.h>
#include <copper.h>
#include <fx.h>
#include <system/memory.h>

#define WIDTH 320
#define HEIGHT 256
#define DEPTH 5
#define SIZE 128

static BitmapT *screen[2];
static u_short active = 0;

static BitmapT *carry;
static BitmapT *buffer;
static CopInsT *bplptr[2][DEPTH];
static CopListT *cp;

#include "data/blurred-pal-1.c"
#include "data/blurred-pal-2.c"
#include "data/blurred-clip.c"

static void Load(void) {
  screen[0] = NewBitmap(WIDTH, HEIGHT, DEPTH);
  screen[1] = NewBitmap(WIDTH, HEIGHT, DEPTH);
}

static void UnLoad(void) {
  DeleteBitmap(screen[0]);
  DeleteBitmap(screen[1]);
}

static short iterCount = 0;

static void MakeCopperList(CopListT *cp) {
  short i;

  CopInit(cp);
  CopSetupBitplanes(cp, bplptr[active], screen[active], DEPTH);
  CopWait(cp, Y(-18), 0);
  CopLoadPal(cp, &blurred_1_pal, 0);
  CopWait(cp, Y(127), 0);
  CopMove16(cp, dmacon, DMAF_RASTER);
  CopLoadPal(cp, &blurred_2_pal, 0);
  CopWait(cp, Y(128), 0);
  CopMove16(cp, dmacon, DMAF_SETCLR | DMAF_RASTER);
  for (i = 0; i < DEPTH; i++)
    bplptr[1][i] = CopMove32(cp, bplpt[i], screen[active]->planes[i] - WIDTH / 16);
  CopEnd(cp);
}

static void Init(void) {
  short i;

  EnableDMA(DMAF_BLITTER | DMAF_BLITHOG);

  for (i = 0; i < 2; i++) {
    BitmapClear(screen[i]);
    WaitBlitter();

    /* Make the center of blurred shape use colors from range 16-31. */
    CircleEdge(screen[i], 4, SIZE / 2 + 16, SIZE / 2, SIZE / 4 - 1);
    BlitterFill(screen[i], 4);

    BitmapCopy(screen[i], WIDTH / 2, 0, &clip);
  }

  buffer = NewBitmap(SIZE, SIZE, 4);
  carry = NewBitmap(SIZE, SIZE, 2);

  SetupPlayfield(MODE_LORES, DEPTH, X(0), Y(0), WIDTH, HEIGHT);

  cp = NewCopList(200);
  MakeCopperList(cp);
  CopListActivate(cp);
  EnableDMA(DMAF_RASTER);
}

static void Kill(void) {
  DisableDMA(DMAF_COPPER | DMAF_RASTER | DMAF_BLITTER | DMAF_BLITHOG);

  DeleteCopList(cp);
  DeleteBitmap(carry);
  DeleteBitmap(buffer);
}

static void RotatingTriangle(short t, short phi, short size) {
  Point2D p[3];
  short i, j;

  /* Calculate vertices of a rotating triangle. */
  for (i = 0; i < 3; i++) {
    short k = SIN(t + phi) / 2;
    short x = SIN(k + i * (SIN_PI * 2 / 3));
    short y = COS(k + i * (SIN_PI * 2 / 3));

    p[i].x = normfx((short)size * (short)x) / 2 + SIZE / 2;
    p[i].y = normfx((short)size * (short)y) / 2 + SIZE / 2;
  }

  /* Create a bob with rotating triangle. */
  for (i = 0, j = 1; i < 3; i++, j = (i == 2 ? 0 : i + 1))
    BlitterLine(p[i].x, p[i].y, p[j].x, p[j].y);
}

static void DrawShape(void) {
  BlitterClear(carry, 0);
  BlitterLineSetup(carry, 0, LINE_EOR|LINE_ONEDOT);

  RotatingTriangle(iterCount * 16, 0, SIZE - 1);
  RotatingTriangle(iterCount * 16, SIN_PI * 2 / 3, SIZE - 1);
  RotatingTriangle(-iterCount * 16, SIN_PI * 2 / 3, SIZE / 2 - 1);

  BlitterFill(carry, 0);
}

PROFILE(BlurredRender);

static void Render(void) {
  ProfilerStart(BlurredRender);
  {
    if (iterCount++ & 1)
      BitmapDecSaturated(buffer, carry);
    DrawShape();
    BitmapIncSaturated(buffer, carry);
    BitmapCopy(screen[active], 16, 0, buffer);
  }
  ProfilerStop(BlurredRender);
  

  ITER(i, 0, DEPTH - 1, {
    CopInsSet32(bplptr[0][i], screen[active]->planes[i]);
    CopInsSet32(bplptr[1][i], screen[active]->planes[i] - WIDTH / 16);
    });

  TaskWaitVBlank();
  active ^= 1;
}

EFFECT(Blurred, Load, UnLoad, Init, Kill, Render, NULL);
