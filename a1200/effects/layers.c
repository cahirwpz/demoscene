#include <stdlib.h>

#include "std/debug.h"
#include "std/fastmath.h"
#include "std/math.h"
#include "std/memory.h"

#include "gfx/ellipse.h"
#include "gfx/layers.h"
#include "gfx/rectangle.h"
#include "gfx/png.h"
#include "tools/frame.h"
#include "tools/loopevent.h"
#include "tools/profiling.h"

#include "system/c2p.h"
#include "system/display.h"
#include "system/vblank.h"

const int WIDTH = 320;
const int HEIGHT = 200;
const int DEPTH = 8;

static PixBufT *canvas;
static PixBufT *layerMap;
static PixBufT *image[3];
static PaletteT *imagePal[3];

void AcquireResources() {
  LoadPngImage(&image[0], &imagePal[0], "data/last-hope-64.png");
  LoadPngImage(&image[1], &imagePal[1], "data/bus-stop-64.png");
  LoadPngImage(&image[2], &imagePal[2], "data/dragon-128.png");
}

void ReleaseResources() {
  int i;

  for (i = 0; i < 3; i++) {
    MemUnref(image[i]);
    MemUnref(imagePal[i]);
  }
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
  canvas = NewPixBuf(PIXBUF_CLUT, WIDTH, HEIGHT);
  layerMap = NewPixBuf(PIXBUF_GRAY, WIDTH, HEIGHT);

  LinkPalettes(imagePal[0], imagePal[1], imagePal[2], NULL);
  LoadPalette(imagePal[0]);

  PixBufRemap(image[1], imagePal[1]);
  PixBufRemap(image[2], imagePal[2]);

  StartProfiling();
}

/*
 * Tear down effect function.
 */
void TearDownEffect() {
  StopProfiling();

  UnlinkPalettes(imagePal[0]);
  MemUnref(canvas);
  MemUnref(layerMap);
}

/*
 * Effect rendering functions.
 */
static int Effect = 0;
static const int LastEffect = 1;

void RenderChunky(int frameNumber) {
  int x = 160.0 * sin(M_PI * frameNumber / 100);
  int y = 100.0 * sin(M_PI * frameNumber / 100);

  PROFILE(DrawComposeMap)
    switch (Effect) {
      case 0:
        PixBufClear(layerMap);
        layerMap->fgColor = 2;
        DrawRectangle(layerMap, 0, 0, 160 + x, 100 + y);
        DrawRectangle(layerMap, 160 - x, 100 - y, 320, 200);
        layerMap->fgColor = 1;
        DrawEllipse(layerMap, 160, 100, -x, -y);
        break;
      
      default:
        break;
    }

  PROFILE(LayersCompose)
    LayersCompose(canvas, layerMap, image, 3);

  PROFILE(ChunkyToPlanar)
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

    if (event == LOOP_NEXT)
      Effect = (Effect + 1) % LastEffect;
    if (event == LOOP_PREV) {
      Effect--;
      if (Effect < 0)
        Effect += LastEffect;
    }

    RenderChunky(frameNumber);
    RenderFrameNumber(frameNumber);
    RenderFramesPerSecond(frameNumber);

    DisplaySwap();
  } while ((event = ReadLoopEvent()) != LOOP_EXIT);
}
