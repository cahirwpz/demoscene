#include "std/debug.h"
#include "std/memory.h"
#include "std/resource.h"

#include "gfx/palette.h"
#include "tools/frame.h"
#include "tools/loopevent.h"
#include "tools/profiling.h"

#include "system/c2p.h"
#include "system/display.h"
#include "system/fileio.h"
#include "system/vblank.h"

#include "uvmap/misc.h"
#include "uvmap/render.h"

const int WIDTH = 320;
const int HEIGHT = 256;
const int DEPTH = 8;

static const int maps = 11;
static int lastMap = -1;

void ChangeMap(int newMap) {
  while (newMap < 0)
    newMap += maps;

  newMap = newMap % maps;

  if (newMap != lastMap) {
    UVMapT *map = R_("Map");

    switch (newMap) {
      case 0:
        UVMapGenerate0(map);
        break;
      case 1:
        UVMapGenerate1(map);
        break;
      case 2:
        UVMapGenerate2(map);
        break;
      case 3:
        UVMapGenerate3(map);
        break;
      case 4:
        UVMapGenerate4(map);
        break;
      case 5:
        UVMapGenerate5(map);
        break;
      case 6:
        UVMapGenerate6(map);
        break;
      case 7:
        UVMapGenerate7(map);
        break;
      case 8:
        UVMapGenerate8(map);
        break;
      case 9:
        UVMapGenerate9(map);
        break;
      case 10:
        UVMapGenerate10(map);
        break;
      default:
        break;
    }

    UVMapSetTexture(map, R_("Texture"));

    lastMap = newMap;
  }
}

/*
 * Set up resources.
 */
void AddInitialResources() {
  ResAdd("Texture", NewPixBufFromFile("data/texture.8"));
  ResAdd("TexturePal", NewPaletteFromFile("data/texture.pal"));
  ResAdd("Map", NewUVMap(WIDTH, HEIGHT, UV_FAST, 256, 256));
  ResAdd("Canvas", NewPixBuf(PIXBUF_CLUT, WIDTH, HEIGHT));
}

/*
 * Set up display function.
 */
bool SetupDisplay() {
  ChangeMap(0);
  return InitDisplay(WIDTH, HEIGHT, DEPTH);
}

/*
 * Set up effect function.
 */
void SetupEffect() {
  LoadPalette(R_("TexturePal"));
  StartProfiling();
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
void RenderChunky(int frameNumber) {
  PixBufT *canvas = R_("Canvas");

  int du = 2 * frameNumber;
  int dv = 4 * frameNumber;

  UVMapSetOffset(R_("Map"), du, dv);
  PROFILE(UVMapRender)
    UVMapRender(R_("Map"), canvas);
  PROFILE(C2P)
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
      ChangeMap(lastMap + 1);
    if (event == LOOP_PREV)
      ChangeMap(lastMap - 1);

    RenderChunky(frameNumber);
    RenderFrameNumber(frameNumber);
    RenderFramesPerSecond(frameNumber);

    DisplaySwap();
  } while ((event = ReadLoopEvent()) != LOOP_EXIT);
}
