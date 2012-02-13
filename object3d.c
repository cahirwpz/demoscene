#include <math.h>

#include "engine/object.h"
#include "gfx/line.h"
#include "gfx/transformations.h"
#include "gfx/triangle.h"
#include "std/resource.h"

#include "system/c2p.h"
#include "system/debug.h"
#include "system/display.h"
#include "system/memory.h"
#include "system/vblank.h"

#include "frame_tools.h"

const int WIDTH = 320;
const int HEIGHT = 256;
const int DEPTH = 8;

static CanvasT *Canvas;

/*
 * Set up display function.
 */
bool SetupDisplay() {
  return InitDisplay(WIDTH, HEIGHT, DEPTH);
}

/*
 * Set up effect function.
 */
void SetupEffect() {
  Canvas = NewCanvas(WIDTH, HEIGHT);
  CanvasFill(Canvas, 0);

  TS_Init();
}

/*
 * Tear down effect function.
 */
void TearDownEffect() {
  TS_End();

  DeleteCanvas(Canvas);
}

/*
 * Effect rendering functions.
 */
void RenderVector(int frameNumber) {
}

void RenderChunky(int frameNumber) {
  c2p1x1_8_c5_bm(GetCanvasPixelData(Canvas), GetCurrentBitMap(), WIDTH, HEIGHT, 0, 0);
}

/*
 * Main loop.
 */
void MainLoop() {
  SetVBlankCounter(0);

  while (GetVBlankCounter() < 500) {
    int frameNumber = GetVBlankCounter();

    RenderVector(frameNumber);
    RenderChunky(frameNumber);
    RenderFrameNumber(frameNumber);

    DisplaySwap();
  }
}
