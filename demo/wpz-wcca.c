#include "std/debug.h"
#include "std/math.h"
#include "std/memory.h"
#include "std/resource.h"

#include "audio/stream.h"
#include "uvmap/generate.h"
#include "uvmap/render.h"
#include "gfx/blit.h"
#include "gfx/colors.h"
#include "gfx/palette.h"

#include "system/audio.h"
#include "system/c2p.h"
#include "system/display.h"

#include "config.h"
#include "demo.h"
#include "timeline.h"

const int WIDTH = 320;
const int HEIGHT = 256;
const int DEPTH = 8;

const char *DemoConfigPath = "wpz-wcca.json";

static AudioStreamT *TheMusic = NULL;
static PixBufT *TheLoadImg = NULL;
static PaletteT *TheLoadPal = NULL;

PARAMETER(PixBufT *, TheCanvas, NULL);
PARAMETER(PaletteT *, ThePalette, NULL);

/*
 * Load demo.
 */
bool LoadDemo() {
  const char *loadImgPath = JsonQueryString(DemoConfig, "load/image");
  const char *loadPalPath = JsonQueryString(DemoConfig, "load/palette");
  const char *musicPath = JsonQueryString(DemoConfig, "music/file");

  if ((TheLoadImg = NewPixBufFromFile(loadImgPath)) &&
      (TheLoadPal = NewPaletteFromFile(loadPalPath)) &&
      (TheMusic = AudioStreamOpen(musicPath)) &&
      (TheCanvas = NewPixBuf(PIXBUF_CLUT, WIDTH, HEIGHT)) &&
      InitDisplay(WIDTH, HEIGHT, DEPTH))
  {
    c2p1x1_8_c5_bm(TheLoadImg->data, GetCurrentBitMap(), WIDTH, HEIGHT, 0, 0);
    LoadPalette(TheLoadPal);
    DisplaySwap();

    return true;
  }

  return false;
}

/*
 * Transition to demo.
 */
void BeginDemo() {
  /* Release loading screen image... */
  MemUnref(TheLoadImg);
  MemUnref(TheLoadPal);

  /* Play the audio stream... */
  AudioStreamPlay(TheMusic);
}

/*
 * Tear down demo.
 */
void KillDemo() {
  AudioStreamStop(TheMusic);

  MemUnref(TheMusic);
  MemUnref(TheCanvas);
}

/*
 * User updated the time.
 */
void DemoUpdateTime(int oldFrameNumber, int newFrameNumber) {
  AudioStreamUpdatePos(TheMusic);
}

/*
 * Set up resources.
 */
void SetupResources() {
}

/*****************************************************************************/

PARAMETER(PixBufT *, ClipartImg, NULL);
PARAMETER(PaletteT *, ClipartPal, NULL);
PARAMETER(int, ClipartX, 0);
PARAMETER(int, ClipartY, 0);

CALLBACK(BlitClipartToCanvas) {
  PixBufBlit(TheCanvas, ClipartX, ClipartY, ClipartImg, NULL);
}

CALLBACK(ClearCanvas) {
  PixBufClear(TheCanvas);
}

CALLBACK(ReloadPalette) {
  LoadPalette(ThePalette);
}

CALLBACK(RenderCanvas) {
  c2p1x1_8_c5_bm(TheCanvas->data, GetCurrentBitMap(), WIDTH, HEIGHT, 0, 0);
}

CALLBACK(FeedAudioStream) {
  AudioStreamFeed(TheMusic);
}

#include "wpz-wcca.syms"
