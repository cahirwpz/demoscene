#include <stdlib.h>

#include "std/debug.h"
#include "std/fastmath.h"
#include "std/math.h"
#include "std/memory.h"
#include "std/resource.h"

#include "gfx/blit.h"
#include "tools/frame.h"
#include "tools/gradient.h"
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
  ResAdd("Canvas", NewPixBuf(PIXBUF_CLUT, WIDTH, HEIGHT));
  ResAdd("Map1", NewPixBuf(PIXBUF_GRAY, WIDTH, HEIGHT));
  ResAdd("Map2", NewPixBuf(PIXBUF_GRAY, WIDTH, HEIGHT));
  ResAdd("Image", NewPixBufFromFile("data/samkaat-absinthe.8"));
  ResAdd("ImagePal", NewPaletteFromFile("data/samkaat-absinthe.pal"));
  ResAdd("Darken", NewPixBufFromFile("data/samkaat-absinthe-darken.8"));
  ResAdd("Lighten", NewPixBufFromFile("data/samkaat-absinthe-lighten.8"));
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

  int change = (frameNumber * 2) % 256;

  switch (Effect) {
    case 0:
      map = R_("Map1");
      PixBufCopy(canvas, image);
      PixBufSetColorMap(map, R_("Lighten"), change);
      PixBufSetBlitMode(map, BLIT_COLOR_MAP);
      PixBufBlit(canvas, 0, 0, map, NULL);
      break;

    case 1:
      map = R_("Map1");
      PixBufCopy(canvas, image);
      PixBufSetColorMap(map, R_("Darken"), change - 64);
      PixBufSetBlitMode(map, BLIT_COLOR_MAP);
      PixBufBlit(canvas, 0, 0, map, NULL);
      break;

    case 2:
      map = R_("Map2");
      PixBufCopy(canvas, image);
      PixBufSetColorMap(map, R_("Lighten"), change - 64);
      PixBufSetBlitMode(map, BLIT_COLOR_MAP);
      PixBufBlit(canvas, 0, 0, map, NULL);
      break;

    case 3:
      map = R_("Map2");
      PixBufCopy(canvas, image);
      PixBufSetColorMap(map, R_("Darken"), change - 64);
      PixBufSetBlitMode(map, BLIT_COLOR_MAP);
      PixBufBlit(canvas, 0, 0, map, NULL);
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

    DisplaySwap();
  } while ((event = ReadLoopEvent()) != LOOP_EXIT);
}
