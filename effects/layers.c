#include <stdlib.h>

#include "std/debug.h"
#include "std/fastmath.h"
#include "std/math.h"
#include "std/memory.h"
#include "std/resource.h"

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

/*
 * Set up resources.
 */
void AddInitialResources() {
  ResAdd("Canvas", NewPixBuf(PIXBUF_CLUT, WIDTH, HEIGHT));
  ResAdd("LayerMap", NewPixBuf(PIXBUF_GRAY, WIDTH, HEIGHT));
  ResAddPngImage("Image1", "Image1Pal", "data/last-hope-64.png");
  ResAddPngImage("Image2", "Image2Pal", "data/bus-stop-64.png");
  ResAddPngImage("Image3", "Image3Pal", "data/dragon-128.png");
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
  LinkPalettes(R_("Image1Pal"), R_("Image2Pal"), R_("Image3Pal"), NULL);
  LoadPalette(R_("Image1Pal"));

  PixBufRemap(R_("Image2"), R_("Image2Pal"));
  PixBufRemap(R_("Image3"), R_("Image3Pal"));

  StartProfiling();
}

/*
 * Tear down effect function.
 */
void TearDownEffect() {
  UnlinkPalettes(R_("Image1Pal"));
  StopProfiling();
}

/*
 * Effect rendering functions.
 */
static int Effect = 0;
static const int LastEffect = 1;

void RenderChunky(int frameNumber) {
  PixBufT *canvas = R_("Canvas");
  PixBufT *map = R_("LayerMap");
  PixBufT *image[3] = { R_("Image1"), R_("Image2"), R_("Image3") };

  int x = 160.0 * sin(M_PI * frameNumber / 100);
  int y = 100.0 * sin(M_PI * frameNumber / 100);

  PROFILE(DrawComposeMap)
    switch (Effect) {
      case 0:
        PixBufClear(map);
        map->fgColor = 2;
        DrawRectangle(map, 0, 0, 160 + x, 100 + y);
        DrawRectangle(map, 160 - x, 100 - y, 320, 200);
        map->fgColor = 1;
        DrawEllipse(map, 160, 100, -x, -y);
        break;
      
      default:
        break;
    }

  PROFILE(LayersCompose)
    LayersCompose(canvas, map, image, 3);

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
