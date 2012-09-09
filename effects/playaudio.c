#include "std/debug.h"
#include "std/memory.h"
#include "std/resource.h"

#include "audio/stream.h"
#include "gfx/hsl.h"
#include "gfx/line.h"
#include "gfx/palette.h"
#include "tools/frame.h"

#include "system/c2p.h"
#include "system/display.h"
#include "system/input.h"
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
  bool finish = FALSE;

  AudioStreamPlay(audio);

  SetVBlankCounter(0);

  do {
    int frameNumber = GetVBlankCounter();

    RenderChunky(frameNumber);
    RenderFrameNumber(frameNumber);

    DisplaySwap();

    AudioStreamFeedIfHungry(audio);

    {
      InputEventT event; 

      while (EventQueuePop(&event)) {
        switch (event.ie_Class) {
          case IECLASS_RAWKEY:
            if (event.ie_Code & IECODE_UP_PREFIX) {
              switch (event.ie_Code & ~IECODE_UP_PREFIX) {
                case KEY_UP:
                  ChangeVBlankCounter(-10 * FRAMERATE);
                  AudioStreamRewind(audio);
                  break;
                case KEY_DOWN:
                  ChangeVBlankCounter(10 * FRAMERATE);
                  AudioStreamRewind(audio);
                  break;
                case KEY_LEFT:
                  ChangeVBlankCounter(-FRAMERATE);
                  AudioStreamRewind(audio);
                  break;
                case KEY_RIGHT:
                  ChangeVBlankCounter(FRAMERATE);
                  AudioStreamRewind(audio);
                  break;
                case KEY_ESCAPE:
                  finish = TRUE;
                  break;
                default:
                  break;
              }
            }

          default:
            break;
        }
      }
    }
  } while (!finish);

  AudioStreamStop(audio);
}
