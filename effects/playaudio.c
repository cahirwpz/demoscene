#include "std/debug.h"
#include "std/math.h"
#include "std/memory.h"
#include "std/resource.h"

#include "audio/stream.h"
#include "gfx/blit.h"
#include "gfx/line.h"
#include "gfx/palette.h"
#include "gfx/png.h"
#include "tools/frame.h"
#include "uvmap/generate.h"
#include "uvmap/render.h"

#include "system/audio.h"
#include "system/c2p.h"
#include "system/display.h"
#include "system/input.h"
#include "system/vblank.h"

const int WIDTH = 320;
const int HEIGHT = 256;
const int DEPTH = 8;

UVMapGenerate(Polar, (1.0f - r * 0.7f), a);

/*
 * Set up resources.
 */
void AddInitialResources() {
  ResAddPngImage("Image", "ImagePal", "data/samkaat-absinthe.png");
  ResAddPngImage("Darken", NULL, "data/samkaat-absinthe-darken.png");
  ResAdd("Texture", NewPixBuf(PIXBUF_GRAY, 256, 256));
  ResAdd("Shade", NewPixBuf(PIXBUF_GRAY, WIDTH, HEIGHT));
  ResAdd("Map", NewUVMap(WIDTH, HEIGHT, UV_FAST, 256, 256));
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
  UVMapGeneratePolar(R_("Map"));
  LoadPalette(R_("ImagePal"));
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
  PixBufT *texture = R_("Texture");
  PixBufT *shade = R_("Shade");
  AudioBufferT *buffer = AudioStreamGetBuffer(R_("Audio"));
  static int array[256];
  int i, j;

  PixBufClear(texture);

  for (i = 0; i < min(buffer->length, texture->width); i++) {
    int y1 = abs((int8_t)buffer->left.hi[i]);
    int y2 = abs((int8_t)buffer->right.hi[i]);
    int y = y1 + y2;

    if (y > 255)
      y = 255;

    array[i] = (array[(i - 1) & 0xff] + y) / 2;
  }

  for (i = 0; i < min(buffer->length, texture->width); i++) {
    texture->fgColor = 255;
  }

  PixBufCopy(canvas, R_("Image"));

  UVMapSetTexture(R_("Map"), R_("Texture"));
  UVMapSetOffset(R_("Map"), 0, 0);
  UVMapRender(R_("Map"), shade);

  PixBufSetColorMap(shade, R_("Darken"));
  PixBufSetBlitMode(shade, BLIT_COLOR_MAP);
  PixBufBlit(canvas, 0, 0, shade, NULL);

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
