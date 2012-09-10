#include <stdlib.h>

#include "std/debug.h"
#include "std/fastmath.h"
#include "std/math.h"
#include "std/memory.h"
#include "std/resource.h"

#include "distort/scaling.h"
#include "engine/plane.h"
#include "engine/sphere.h"
#include "engine/vector3d.h"
#include "gfx/blit.h"
#include "gfx/canvas.h"
#include "gfx/ellipse.h"
#include "tools/frame.h"
#include "tools/loopevent.h"
#include "txtgen/procedural.h"

#include "system/c2p.h"
#include "system/display.h"
#include "system/vblank.h"

const int WIDTH = 320;
const int HEIGHT = 256;
const int DEPTH = 8;

/*
 * Set up resources.
 */
void AddInitialResources() {
  ResAdd("Canvas", NewCanvas(WIDTH, HEIGHT));
  ResAdd("Map", NewTable(int, WIDTH * HEIGHT));
  ResAdd("Image", NewPixBufFromFile("data/samkaat-absinthe.8"));
  ResAdd("ImagePal", NewPaletteFromFile("data/samkaat-absinthe.pal"));
  ResAdd("ColorMap", NewPixBufFromFile("data/samkaat-absinthe.map"));
}

/*
 * Set up display function.
 */
bool SetupDisplay() {
  return InitDisplay(WIDTH, HEIGHT, DEPTH);
}

/*
 * Set up effect function.
 */
typedef struct FLine {
  float x, y, dx, dy;
  float invNormalLength;
} FLineT;

void FLineInitFromPoints(FLineT *line, FPointT *p0, FPointT *p1) {
  float dx = p1->x - p0->x;
  float dy = p1->y - p0->y;

  line->x = p0->x;
  line->y = p0->y;
  line->dx = dx;
  line->dy = dy;
  line->invNormalLength = FastInvSqrt(dx * dx + dy * dy);
}

static inline float PointToLineDistance(FLineT *line, float px, float py) {
  return fabsf((px - line->x) * line->dy - (py - line->x) * line->dx) *
    line->invNormalLength;
}

void RenderLine() {
  int *map = R_("Map");
  float x, y;
  FLineT line;

  {
    FPointT pa = { 40, 60 };
    FPointT pb = { 200, 140 };

    FLineInitFromPoints(&line, &pa, &pb);
  }

  for (y = 0; y < HEIGHT; y += 1.0f)
    for (x = 0; x < WIDTH; x += 1.0f)
      *map++ = lroundf(PointToLineDistance(&line, x, y));
}

void SetupEffect() {
  RenderLine();
  LoadPalette(R_("ImagePal"));
}

/*
 * Tear down effect function.
 */
void TearDownEffect() {
}

/*
 * Effect rendering functions.
 */
void RenderChunky(int frameNumber) {
  CanvasT *canvas = R_("Canvas");
  int *map = R_("Map");
  PixBufT *image = R_("Image");
  PixBufT *colorMap = R_("ColorMap");

  {
    uint8_t *img = image->data;
    uint8_t *c = colorMap->data;
    uint8_t *d = GetCanvasPixelData(canvas);
    int i;

    for (i = 0; i < WIDTH * HEIGHT; i++) {
      int pixel = img[i];
      int shade = map[i];

      shade += (frameNumber * 2) % 256;

      if (shade < 0)
        shade = 0;
      if (shade > 255)
        shade = 255;

      d[i] = c[pixel * 256 + shade];
    }
  }

  c2p1x1_8_c5_bm(GetCanvasPixelData(R_("Canvas")),
                 GetCurrentBitMap(), WIDTH, HEIGHT, 0, 0);
}

/*
 * Main loop.
 */
void MainLoop() {
  SetVBlankCounter(0);

  do {
    int frameNumber = GetVBlankCounter();

    RenderChunky(frameNumber);
    RenderFrameNumber(frameNumber);

    DisplaySwap();
  } while (ReadLoopEvent() != LOOP_EXIT);
}
