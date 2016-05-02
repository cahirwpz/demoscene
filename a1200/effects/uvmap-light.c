#include "std/debug.h"
#include "std/memory.h"

#include "gfx/blit.h"
#include "gfx/palette.h"
#include "gfx/png.h"
#include "tools/frame.h"
#include "tools/gradient.h"
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
static PixBufT *canvas;
static PixBufT *shades;
static PixBufT *texture;
static PixBufT *colorMap;
static PixBufT *component;
static PaletteT *texturePal;
static uint8_t *colorFunc;

void AcquireResources() {
  LoadPngImage(&texture, &texturePal, "data/texture-shades.png");
  LoadPngImage(&colorMap, NULL, "data/texture-shades-map.png");
}

void ReleaseResources() {
}

bool SetupDisplay() {
  return InitDisplay(WIDTH, HEIGHT, DEPTH);
}

void SetupEffect() {
  uvmap = NewUVMap(WIDTH, HEIGHT, UV_FAST, 256, 256);
  shades = NewPixBuf(PIXBUF_GRAY, WIDTH, HEIGHT);
  canvas = NewPixBuf(PIXBUF_CLUT, WIDTH, HEIGHT);
  colorFunc = NewTable(uint8_t, 256);

  LoadPalette(texturePal);

  UVMapGenerate4(uvmap);
  UVMapSetTexture(uvmap, texture);
  uvmap->lightMap = shades;
  PixBufSetColorMap(shades, colorMap);

  component = NewPixBufWrapper(WIDTH, HEIGHT, uvmap->map.fast.u);

  StartProfiling();
}

void TearDownEffect() {
  StopProfiling();
}

void RenderEffect(int frameNumber) {
  int du = 2 * frameNumber;
  int dv = 4 * frameNumber;

  {
    int i;

    for (i = 0; i < 256; i++) {
      uint8_t v = i + du;

      if (v >= 128)
        colorFunc[i] = ~v * 2;
      else 
        colorFunc[i] = v * 2;
    }
  }

  PixBufSetBlitMode(component, BLIT_COLOR_FUNC);
  PixBufSetColorFunc(component, colorFunc);
  PROFILE (PixBufBlitWithColorFunc)
    PixBufBlit(shades, 0, 0, component, NULL);

  UVMapSetOffset(uvmap, du, dv);
  PROFILE (UVMapRenderFast)
    UVMapRender(uvmap, canvas);

  c2p1x1_8_c5_bm(canvas->data, GetCurrentBitMap(), WIDTH, HEIGHT, 0, 0);
}

void MainLoop() {
  LoopEventT event = LOOP_CONTINUE;

  SetVBlankCounter(0);

  do {
    int frameNumber = GetVBlankCounter();

    RenderEffect(frameNumber);
    RenderFrameNumber(frameNumber);
    RenderFramesPerSecond(frameNumber);

    DisplaySwap();
  } while ((event = ReadLoopEvent()) != LOOP_EXIT);
}
