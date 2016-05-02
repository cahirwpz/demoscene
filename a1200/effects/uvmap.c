#include "std/debug.h"
#include "std/memory.h"

#include "gfx/palette.h"
#include "gfx/png.h"
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

static UVMapT *uvmap;
static PixBufT *texture;
static PixBufT *canvas;
static PixBufT *texture;
static PaletteT *texturePal;

static const int maps = 11;
static int lastMap = -1;
static void (*generate[])(UVMapT *) = {
  UVMapGenerate0, UVMapGenerate1, UVMapGenerate2, UVMapGenerate3,
  UVMapGenerate4, UVMapGenerate5, UVMapGenerate6, UVMapGenerate7,
  UVMapGenerate8, UVMapGenerate9, UVMapGenerate10
};

void ChangeMap(int newMap) {
  while (newMap < 0)
    newMap += maps;

  newMap = newMap % maps;

  if (newMap != lastMap) {
    generate[newMap](uvmap);

    UVMapSetTexture(uvmap, texture);

    lastMap = newMap;
  }
}

void AcquireResources() {
  LoadPngImage(&texture, &texturePal, "data/texture.png");
}

void ReleaseResources() {
  MemUnref(texture);
  MemUnref(texturePal);
}

bool SetupDisplay() {
  return InitDisplay(WIDTH, HEIGHT, DEPTH);
}

void SetupEffect() {
  uvmap = NewUVMap(WIDTH, HEIGHT, UV_FAST, 256, 256);
  canvas = NewPixBuf(PIXBUF_CLUT, WIDTH, HEIGHT);

  ChangeMap(0);
  LoadPalette(texturePal);
}

void TearDownEffect() {
  MemUnref(uvmap);
  MemUnref(canvas);
}

void RenderChunky(int frameNumber) {
  int du = 2 * frameNumber;
  int dv = 4 * frameNumber;

  UVMapSetOffset(uvmap, du, dv);
  PROFILE(UVMapRender)
    UVMapRender(uvmap, canvas);
  PROFILE(C2P)
    c2p1x1_8_c5_bm(canvas->data, GetCurrentBitMap(), WIDTH, HEIGHT, 0, 0);
}

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
