#include <math.h>

#include "std/debug.h"
#include "std/memory.h"
#include "std/resource.h"

#include "gfx/ellipse.h"
#include "gfx/line.h"
#include "gfx/ms2d.h"
#include "gfx/triangle.h"
#include "tools/frame.h"
#include "tools/loopevent.h"

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
  static PointT Cross[] = {
    {1, 0}, {2, 0}, {2, 1}, {3, 1}, {3, 2}, {2, 2}, {2, 3}, {1, 3}, {1, 2}, {0, 2}, {0, 1}, {1, 1}
  };
  static PointT CrossToDraw[12];

  static PointT Triangle[] = { {-15, -10}, {10, -5}, {0, 20} };
  static PointT TriangleToDraw[3];

  ResAddStatic("Cross", Cross);
  ResAddStatic("CrossToDraw", CrossToDraw);
  ResAddStatic("Triangle", Triangle);
  ResAddStatic("TriangleToDraw", TriangleToDraw);

  ResAdd("Canvas", NewPixBuf(PIXBUF_CLUT, WIDTH, HEIGHT));
  ResAdd("ms2d", NewMatrixStack2D());
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
void SetupEffect() {
  PixBufClear(R_("Canvas"));
}

/*
 * Tear down effect function.
 */
void TearDownEffect() {
}

/*
 * Effect rendering functions.
 */
int effect = 0;
const int lastEffect = 3;

void RenderVector(int frameNumber) {
  PixBufT *canvas = R_("Canvas");
  PointT *toDraw = R_("TriangleToDraw");
  MatrixStack2D *ms = R_("ms2d");

  float s = sin(frameNumber * 3.14159265f / 22.5f);
  float c = cos(frameNumber * 3.14159265f / 45.0f);

  StackReset(ms);
  PushTranslation2D(ms, -1.5f, -1.5f);
  PushScaling2D(ms, 20.0f + 10.0f * s, 20.0f + 10.0f * s);
  PushRotation2D(ms, (float)(frameNumber * -3));
  PushTranslation2D(ms, (float)(WIDTH/2) + c * (WIDTH/4), (float)(HEIGHT/2));

  Transform2D(R_("CrossToDraw"), R_("Cross"), 12, GetMatrix2D(ms, 0));

  if (effect == 0) {
    PixBufClear(canvas);
    DrawPolyLine(canvas, R_("CrossToDraw"), 12, TRUE);
  }

  StackReset(ms);
  PushTranslation2D(ms, 5.0f, 10.0f);
  PushScaling2D(ms, 2.5f, 2.5f);
  PushRotation2D(ms, (float)(frameNumber*5*c));
  PushTranslation2D(ms, WIDTH/2 + c * 50, HEIGHT/2 + s * 20);

  Transform2D(R_("TriangleToDraw"), R_("Triangle"), 3, GetMatrix2D(ms, 0));

  frameNumber &= 255;

  if (frameNumber < 128)
    canvas->fgColor = frameNumber * 2;
  else 
    canvas->fgColor = (255 - frameNumber) * 2;

  if (effect == 1) {
    DrawTriangle(canvas,
                 toDraw[0].x, toDraw[0].y,
                 toDraw[1].x, toDraw[1].y,
                 toDraw[2].x, toDraw[2].y);
  }

  if (effect == 2) {
    DrawEllipse(canvas,
                toDraw[1].x, toDraw[1].y,
                30 + c * 15, 30 + s * 15);
  }

  c2p1x1_8_c5_bm(canvas->data, GetCurrentBitMap(), WIDTH, HEIGHT, 0, 0);
}

/*
 * Main loop.
 */
void MainLoop() {
  LoopEventT event = LOOP_CONTINUE;

  SetVBlankCounter(0);

  do {
    int frameNumber = GetVBlankCounter();

    if (event != LOOP_CONTINUE) {
      PixBufClear(R_("Canvas"));

      if (event == LOOP_NEXT)
        effect = (effect + 1) % lastEffect;
      if (event == LOOP_PREV) {
        effect--;
        if (effect < 0)
          effect += lastEffect;
      }
    }

    RenderVector(frameNumber);
    RenderFrameNumber(frameNumber);

    DisplaySwap();
  } while ((event = ReadLoopEvent()) != LOOP_EXIT);
}
