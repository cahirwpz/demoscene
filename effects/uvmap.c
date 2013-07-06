#include "std/debug.h"
#include "std/memory.h"
#include "std/resource.h"

#include "gfx/palette.h"
#include "tools/frame.h"
#include "tools/gradient.h"
#include "tools/loopevent.h"

#include "system/c2p.h"
#include "system/display.h"
#include "system/fileio.h"
#include "system/vblank.h"

#include "uvmap/misc.h"

const int WIDTH = 320;
const int HEIGHT = 256;
const int DEPTH = 8;

/*
 * Set up resources.
 */
void AddInitialResources() {
  ResAdd("Texture1", NewPixBufFromFile("data/texture-128-01.8"));
  ResAdd("Texture1Pal", NewPaletteFromFile("data/texture-128-01.pal"));
  ResAdd("Texture2", NewPixBufFromFile("data/texture-128-02.8"));
  ResAdd("Texture2Pal", NewPaletteFromFile("data/texture-128-02.pal"));
  ResAdd("Map1", NewUVMap(WIDTH, HEIGHT, UV_OPTIMIZED, 256, 256));
  ResAdd("Map2", NewUVMap(WIDTH, HEIGHT, UV_OPTIMIZED, 256, 256));
  ResAdd("ComposeMap", NewPixBuf(PIXBUF_GRAY, WIDTH, HEIGHT));
  ResAdd("Canvas", NewPixBuf(PIXBUF_CLUT, WIDTH, HEIGHT));
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
  LinkPalettes(R_("Texture1Pal"), R_("Texture2Pal"), NULL);
  LoadPalette(R_("Texture1Pal"));

  PixBufRemap(R_("Texture2"), R_("Texture2Pal"));

  UVMapGenerate0(R_("Map1"));
  UVMapSetTexture(R_("Map1"), R_("Texture1"));

  UVMapGenerate1(R_("Map2"));
  UVMapSetTexture(R_("Map2"), R_("Texture2"));

  {
    FPointT center = { WIDTH / 2, HEIGHT / 2 };
    CircularGradient(R_("ComposeMap"), &center);
  }
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
  PixBufT *canvas = R_("Canvas");

  int du = 2 * frameNumber;
  int dv = 4 * frameNumber;
  int change = (int)(92.0 + 64.0 * sin(M_PI * (float)frameNumber / 64.0));

  UVMapSetOffset(R_("Map1"), du, dv);
  UVMapSetOffset(R_("Map2"), -du, -dv);
  UVMapComposeAndRender(canvas, R_("ComposeMap"), R_("Map1"), R_("Map2"), change);

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

    RenderChunky(frameNumber);
    RenderFrameNumber(frameNumber);

    DisplaySwap();
  } while ((event = ReadLoopEvent()) != LOOP_EXIT);
}
