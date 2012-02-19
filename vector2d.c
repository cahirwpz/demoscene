#include <math.h>

#include "std/debug.h"
#include "std/memory.h"
#include "std/resource.h"

#include "gfx/ellipse.h"
#include "gfx/line.h"
#include "gfx/transformations.h"
#include "gfx/triangle.h"

#include "system/c2p.h"
#include "system/display.h"
#include "system/vblank.h"

#include "frame_tools.h"

const int WIDTH = 320;
const int HEIGHT = 256;
const int DEPTH = 8;

static CanvasT *Canvas;

static PointT *Cross;
static PointT CrossToDraw[12];

static PointT Triangle[] = { {-15, -10}, {10, -5}, {0, 20} };
static PointT TriangleToDraw[3];

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
  Cross = GetResource("cross");
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
  float s = sin(frameNumber * 3.14159265f / 45.0f);
  float c = cos(frameNumber * 3.14159265f / 90.0f);

  TS_Reset();
  TS_PushTranslation2D(-1.5f, -1.5f);
  TS_PushScaling2D(20.0f + 10.0f * s, 20.0f + 10.0f * s);
  TS_PushRotation2D((float)(frameNumber * -3));
  TS_PushTranslation2D((float)(WIDTH/2) + c * (WIDTH/4), (float)(HEIGHT/2));

  M2D_Transform(CrossToDraw, Cross, 12, TS_GetMatrix2D(1));

  //CanvasFill(Canvas, 0);
  //DrawPolyLine(Canvas, CrossToDraw, 12, TRUE);

  TS_Reset();
  TS_PushTranslation2D(5.0f, 10.0f);
  TS_PushScaling2D(2.5f, 2.5f);
  TS_PushRotation2D((float)(frameNumber*5*c));
  TS_PushTranslation2D(WIDTH/2 + c * 50, HEIGHT/2 + s * 20);

  M2D_Transform(TriangleToDraw, Triangle, 3, TS_GetMatrix2D(0));

  frameNumber &= 255;

  if (frameNumber < 128)
    Canvas->fg_col = frameNumber * 2;
  else 
    Canvas->fg_col = (255 - frameNumber) * 2;

  /*
  DrawTriangle(Canvas,
               TriangleToDraw[0].x, TriangleToDraw[0].y,
               TriangleToDraw[1].x, TriangleToDraw[1].y,
               TriangleToDraw[2].x, TriangleToDraw[2].y);
               */
  DrawEllipse(Canvas,
              TriangleToDraw[1].x, TriangleToDraw[1].y,
              30 + c * 15, 30 + s * 15);
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
