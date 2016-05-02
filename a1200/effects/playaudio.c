#include "std/debug.h"
#include "std/math.h"
#include "std/memory.h"

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

static PixBufT *texture;
static PixBufT *image;
static PaletteT *imagePal;
static PixBufT *shade;
static UVMapT *uvmap;
static PixBufT *canvas;
static PixBufT *darken;
static AudioStreamT *audio;

void AcquireResources() {
  LoadPngImage(&image, &imagePal, "data/samkaat-absinthe.png");
  LoadPngImage(&darken, NULL, "data/samkaat-absinthe-darken.png");
  audio = AudioStreamOpen("data/chembro.wav");
}

void ReleaseResources() {
}

bool SetupDisplay() {
  return InitDisplay(WIDTH, HEIGHT, DEPTH) && InitAudio();
}

void SetupEffect() {
  texture = NewPixBuf(PIXBUF_GRAY, 256, 256);
  shade = NewPixBuf(PIXBUF_GRAY, WIDTH, HEIGHT);
  uvmap = NewUVMap(WIDTH, HEIGHT, UV_FAST, 256, 256);
  canvas = NewPixBuf(PIXBUF_CLUT, WIDTH, HEIGHT);

  UVMapGeneratePolar(uvmap);
  LoadPalette(imagePal);
}

void TearDownEffect() {
  KillAudio();
}

void RenderChunky(int frameNumber) {
  AudioBufferT *buffer = AudioStreamGetBuffer(audio);
  static int array[256];
  int i;

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

  PixBufCopy(canvas, image);

  UVMapSetTexture(uvmap, texture);
  UVMapSetOffset(uvmap, 0, 0);
  UVMapRender(uvmap, shade);

  PixBufSetColorMap(shade, darken);
  PixBufSetBlitMode(shade, BLIT_COLOR_MAP);
  PixBufBlit(canvas, 0, 0, shade, NULL);

  c2p1x1_8_c5_bm(canvas->data, GetCurrentBitMap(), WIDTH, HEIGHT, 0, 0);
}

void MainLoop() {
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
