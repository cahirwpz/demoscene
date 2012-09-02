#include "std/debug.h"
#include "std/memory.h"
#include "std/resource.h"

#include "audio/stream.h"
#include "gfx/hsl.h"
#include "gfx/line.h"
#include "gfx/palette.h"
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
  ResAdd("Canvas", NewCanvas(WIDTH, HEIGHT));
  ResAdd("Audio", AudioStreamOpen("data/chembro.snd"));
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

void RenderChunky(int frameNumber) {
  c2p1x1_8_c5_bm(GetCanvasPixelData(R_("Canvas")),
                 GetCurrentBitMap(), WIDTH, HEIGHT, 0, 0);
}

/*
 * Main loop.
 */
void MainLoop() {
  AudioStreamT *audio = R_("Audio");

  AudioStreamPlay(audio);

  SetVBlankCounter(0);

  do {
    int frameNumber = GetVBlankCounter();

    RenderChunky(frameNumber);
    RenderFrameNumber(frameNumber);

    DisplaySwap();

    AudioStreamFeedIfHungry(audio);
  } while (ReadLoopEvent() != LOOP_EXIT);

  AudioStreamStop(audio);
}
