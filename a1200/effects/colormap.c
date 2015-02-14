#include <stdlib.h>

#include "std/debug.h"
#include "std/fastmath.h"
#include "std/math.h"
#include "std/memory.h"
#include "std/resource.h"

#include "gfx/blit.h"
#include "gfx/png.h"
#include "tools/frame.h"
#include "tools/gradient.h"
#include "tools/loopevent.h"
#include "tools/profiling.h"

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
  ResAdd("Canvas", NewPixBuf(PIXBUF_CLUT, WIDTH, HEIGHT));
  ResAdd("Map1", NewPixBuf(PIXBUF_GRAY, WIDTH, HEIGHT));
  ResAdd("Map2", NewPixBuf(PIXBUF_GRAY, WIDTH, HEIGHT));
  ResAddPngImage("Image", "ImagePal", "data/samkaat-absinthe.png");
  ResAddPngImage("Darken", NULL, "data/samkaat-absinthe-darken.png");
  ResAddPngImage("Lighten", NULL, "data/samkaat-absinthe-lighten.png");
  ResAdd("Shade", NewPixBuf(PIXBUF_GRAY, WIDTH, HEIGHT));
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
  StartProfiling();

  {
    FLineT line;
    FPointT pa = { 0, 0 };
    FPointT pb = { WIDTH, HEIGHT };

    FLineInitFromPoints(&line, &pa, &pb);
    LinearGradient(R_("Map1"), &line);
  }

  {
    FPointT center = { WIDTH / 2, HEIGHT / 2 };
    CircularGradient(R_("Map2"), &center);
  }

  LoadPalette(R_("ImagePal"));
}

/*
 * Tear down effect function.
 */
void TearDownEffect() {
  StopProfiling();
}

/*
 * Effect rendering functions.
 */
static int Effect = 0;
static const int LastEffect = 4;

void RenderChunky(int frameNumber) {
  PixBufT *canvas = R_("Canvas");
  PixBufT *image = R_("Image");
  PixBufT *map;
  PixBufT *shade = R_("Shade");

  int change = (frameNumber * 2) % 256;

  switch (Effect) {
    case 0:
      map = R_("Map1");
      PROFILE(PixBufCopy)
        PixBufCopy(canvas, image);
      PROFILE(PixBufAddAndClamp)
        PixBufAddAndClamp(shade, map, change);
      PixBufSetColorMap(shade, R_("Lighten"));
      PixBufSetBlitMode(shade, BLIT_COLOR_MAP);
      PROFILE(PixBufBlit)
        PixBufBlit(canvas, 0, 0, shade, NULL);
      break;

    case 1:
      map = R_("Map1");
      PROFILE(PixBufCopy)
        PixBufCopy(canvas, image);
      PROFILE(PixBufAddAndClamp)
        PixBufAddAndClamp(shade, map, change - 64);
      PixBufSetColorMap(shade, R_("Darken"));
      PixBufSetBlitMode(shade, BLIT_COLOR_MAP);
      PROFILE(PixBufBlit)
        PixBufBlit(canvas, 0, 0, shade, NULL);
      break;

    case 2:
      map = R_("Map2");
      PROFILE(PixBufCopy)
        PixBufCopy(canvas, image);
      PROFILE(PixBufAddAndClamp)
        PixBufAddAndClamp(shade, map, change - 64);
      PixBufSetColorMap(shade, R_("Lighten"));
      PixBufSetBlitMode(shade, BLIT_COLOR_MAP);
      PROFILE(PixBufBlit)
        PixBufBlit(canvas, 0, 0, shade, NULL);
      break;

    case 3:
      map = R_("Map2");
      PROFILE(PixBufCopy)
        PixBufCopy(canvas, image);
      PROFILE(PixBufAddAndClamp)
        PixBufAddAndClamp(shade, map, change - 64);
      PixBufSetColorMap(shade, R_("Darken"));
      PixBufSetBlitMode(shade, BLIT_COLOR_MAP);
      PROFILE(PixBufBlit)
        PixBufBlit(canvas, 0, 0, shade, NULL);
      break;

    default:
      break;
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
