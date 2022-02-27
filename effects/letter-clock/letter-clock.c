#include "effect.h"

#include "bitmap.h"
#include "blitter.h"
#include "circle.h"
#include "copper.h"
#include "fx.h"
#include "palette.h"

#define WIDTH 320
#define HEIGHT 256
#define DEPTH 5
#define CENTER_X (WIDTH / 2)
#define CENTER_Y (HEIGHT / 2 + 25)
#define SMALL_R 20
#define BIG_R 75
#define MAX_PROGRESS 255
#define FIRST_LENS 3
#define SECOND_LENS 4

#include "data/clock-bg.c"
#include "data/clock-palette.c"

static CopListT *cp;
static BitmapT *bckg;
int old_x[2], old_y[2];

// Only one lens is moved at a time. The circular movement is partially
// preprocessed.
static void Circles(short intra_x, short intra_y, short progress, short smallR,
                    short bigR) {
  short by_two = progress & 1;
  short plane = by_two ? FIRST_LENS : SECOND_LENS;
  short trig_arg = (progress * SIN_MASK) >> 8;

  int x = normfx(bigR * COS(trig_arg + by_two * SIN_PI)) + intra_x;
  int y = normfx(bigR * SIN(trig_arg + by_two * SIN_PI)) + intra_y;

  {
    // The initial "old" position of the lenses is set in Init()
    Area2D area = {.x = old_x[by_two] - smallR - 2,
                   .y = old_y[by_two] - smallR - 2,
                   .w = smallR * 2 + 4,
                   .h = smallR * 2 + 4};
    BlitterClearArea(bckg, plane, &area);
  }

  CircleEdge(bckg, plane, x, y, smallR);
  BlitterFill(bckg, plane);

  old_x[by_two] = x;
  old_y[by_two] = y;
}

/*
 * In this function we create areas limited by the lines on left and right.
 * They will be useful during the fill operation from Circles().
 * In the end the colour bars will be places under rectangular "lenses"
 * allowing us to use the whole palette.
 */
static void SetupBoxes(void) {
  // Top colour bar
  BlitterLineSetup(bckg, 3, LINE_OR | LINE_SOLID);
  BlitterLine(253, 8, 253, 43);
  BlitterLine(60, 8, 10, 60);
  BlitterLineSetup(bckg, 4, LINE_OR | LINE_SOLID);
  BlitterLine(253, 8, 253, 43);
  BlitterLine(60, 8, 10, 60);

  // Left colour bar
  BlitterLineSetup(bckg, 3, LINE_OR | LINE_SOLID);
  BlitterLine(50, 65, 50, 195);
  BlitterLine(7, 65, 7, 195);

  // Right colour bar
  BlitterLineSetup(bckg, 4, LINE_OR | LINE_SOLID);
  BlitterLine(305, 65, 305, 195);
  BlitterLine(260, 65, 260, 195);
}

static void Init(void) {
  bckg = NewBitmap(WIDTH, HEIGHT, DEPTH);

  SetupPlayfield(MODE_LORES, DEPTH, X(0), Y(0), WIDTH, HEIGHT);
  LoadPalette(&clock_pal, 0);

  EnableDMA(DMAF_BLITTER | DMAF_BLITHOG);
  BitmapCopy(bckg, 0, 0, &clock_bg);

  cp = NewCopList(80);
  CopInit(cp);
  CopSetupBitplanes(cp, NULL, bckg, DEPTH);
  CopEnd(cp);
  CopListActivate(cp);

  // Clear higher planes (not present in the file) in order to prevent artifacts
  BlitterClear(bckg, 3);
  BlitterClear(bckg, 4);

  SetupBoxes();

  EnableDMA(DMAF_RASTER);

  old_x[0] = CENTER_X;
  old_y[0] = BIG_R + CENTER_Y;
  old_x[1] = CENTER_X;
  old_y[1] = CENTER_Y - BIG_R;
}

static void Kill(void) {
  DisableDMA(DMAF_COPPER | DMAF_RASTER | DMAF_BLITTER);
  DeleteCopList(cp);
  DeleteBitmap(bckg);
}

static void Render(void) {
  Circles(CENTER_X, CENTER_Y, frameCount & MAX_PROGRESS, SMALL_R, BIG_R);
  TaskWaitVBlank();
}

EFFECT(letter_clock, NULL, NULL, Init, Kill, Render);
