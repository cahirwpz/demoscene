#include "std/memory.h"
#include "std/resource.h"

#include "gfx/blit.h"
#include "gfx/filter.h"
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
  ResAdd("Canvas", NewPixBuf(PIXBUF_CLUT, WIDTH, HEIGHT));
  ResAdd("Buffer", NewPixBuf(PIXBUF_CLUT, WIDTH, HEIGHT));
  ResAdd("Image", NewPixBufFromFile("data/samkaat-absinthe.8"));
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
}

/*
 * Tear down effect function.
 */
void TearDownEffect() {
}

/*
 * Effect rendering functions.
 */
static bool Init = true;
static int Effect = 0;
static const int LastEffect = 2;

void RenderEffect(int frameNumber) {
  PixBufT *canvas = R_("Canvas");
  PixBufT *buffer = R_("Buffer");

  if (Init) {
    PixBufCopy(R_("Buffer"), R_("Image"));
    Init = false;
  }

  if (Effect == 0)
    BlurV3(canvas, buffer);
  if (Effect == 1)
    BlurH3(canvas, buffer);

  c2p1x1_8_c5_bm(canvas->data, GetCurrentBitMap(), WIDTH, HEIGHT, 0, 0);

  PixBufSwapData(canvas, buffer);
}

/*
 * Main loop.
 */
void MainLoop() {
  LoopEventT event = LOOP_CONTINUE;

  SetVBlankCounter(0);

  do {
    int frameNumber = GetVBlankCounter();

    if (event == LOOP_NEXT) {
      Effect = (Effect + 1) % LastEffect;
      Init = true;
    }
    if (event == LOOP_PREV) {
      Effect--;
      if (Effect < 0)
        Effect += LastEffect;
      Init = true;
    }

    RenderEffect(frameNumber);
    RenderFrameNumber(frameNumber);

    DisplaySwap();
  } while ((event = ReadLoopEvent()) != LOOP_EXIT);
}
