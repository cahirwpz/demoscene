#include "std/debug.h"
#include "std/memory.h"
#include "std/resource.h"

#include "audio/stream.h"
#include "gfx/hsl.h"
#include "gfx/line.h"
#include "gfx/palette.h"
#include "tools/frame.h"

#include "system/audio.h"
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
  ResAdd("Canvas", NewPixBuf(PIXBUF_CLUT, WIDTH, HEIGHT));
  ResAdd("Audio", AudioStreamOpen("data/chembro.wav"));
}

/*
 * Set up display function.
 */
bool SetupDisplay() {
  return InitDisplay(WIDTH, HEIGHT, DEPTH) && InitAudio();
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
  KillAudio();
}

/*
 * Effect rendering functions.
 */

void RenderChunky(int frameNumber) {
  PixBufT *canvas = R_("Canvas");
  AudioBufferT *buffer = AudioStreamGetBuffer(R_("Audio"));
  int i;

  PixBufClear(canvas);

  for (i = 0; i < min(buffer->length, WIDTH); i++) {
    int y1 = (int8_t)buffer->left.hi[i];
    int y2 = (int8_t)buffer->right.hi[i];

    canvas->fgColor = abs(y1) + 128;
    DrawLine(canvas, i,  64, i,  64 + y1 / 2);
    canvas->fgColor = abs(y2) + 128;
    DrawLine(canvas, i, 192, i, 192 + y2 / 2);
  }

  c2p1x1_8_c5_bm(canvas->data, GetCurrentBitMap(), WIDTH, HEIGHT, 0, 0);
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

    AudioStreamFeed(audio);

    {
      InputEventT event; 

      while (EventQueuePop(&event)) {
        switch (event.ie_Class) {
          case IECLASS_RAWKEY:
            if (event.ie_Code & IECODE_UP_PREFIX) {
              switch (event.ie_Code & ~IECODE_UP_PREFIX) {
                case KEY_UP:
                  ChangeVBlankCounter(-10 * FRAMERATE);
                  AudioStreamUpdatePos(audio);
                  break;
                case KEY_DOWN:
                  ChangeVBlankCounter(10 * FRAMERATE);
                  AudioStreamUpdatePos(audio);
                  break;
                case KEY_LEFT:
                  ChangeVBlankCounter(-FRAMERATE);
                  AudioStreamUpdatePos(audio);
                  break;
                case KEY_RIGHT:
                  ChangeVBlankCounter(FRAMERATE);
                  AudioStreamUpdatePos(audio);
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
