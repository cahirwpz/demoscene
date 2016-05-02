#include "std/debug.h"
#include "std/memory.h"

#include "gfx/blit.h"
#include "gfx/ellipse.h"
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

static PixBufT *texture[2];
static PaletteT *texturePal[2];
static PixBufT *composeMap;
static PixBufT *component;
static UVMapT *uvmap[2];
static PixBufT *canvas;
static uint8_t *colorFunc;

void AcquireResources() {
  LoadPngImage(&texture[0], &texturePal[0], "data/texture-128-01.png");
  LoadPngImage(&texture[1], &texturePal[1], "data/texture-128-02.png");
}

void ReleaseResources() {
}

bool SetupDisplay() {
  return InitDisplay(WIDTH, HEIGHT, DEPTH);
}

void SetupEffect() {
  uvmap[0] = NewUVMap(WIDTH, HEIGHT, UV_FAST, 256, 256);
  uvmap[1] = NewUVMap(WIDTH, HEIGHT, UV_FAST, 256, 256);
  composeMap = NewPixBuf(PIXBUF_GRAY, WIDTH, HEIGHT);
  canvas = NewPixBuf(PIXBUF_CLUT, WIDTH, HEIGHT);
  colorFunc = NewTable(uint8_t, 256);

  LinkPalettes(texturePal[0], texturePal[1], NULL);
  LoadPalette(texturePal[0]);

  PixBufRemap(texture[1], texturePal[1]);

  UVMapGenerate3(uvmap[0]);
  UVMapSetTexture(uvmap[0], texture[0]);

  UVMapGenerate4(uvmap[1]);
  UVMapSetTexture(uvmap[1], texture[1]);

  component = NewPixBufWrapper(WIDTH, HEIGHT, uvmap[1]->map.fast.v);
}

void TearDownEffect() {
  UnlinkPalettes(texturePal[0]);
}

static int EffectNum = 0;

void RenderChunky(int frameNumber) {
  int du = 2 * frameNumber;
  int dv = 4 * frameNumber;

  if (EffectNum == 0) {
    int i;

    for (i = 0; i < 256; i++) {
      int v = 128 - ((frameNumber * 2) % 256 + i);
      colorFunc[i] = (v & 0xff) >= 128 ? 1 : 0;
    }

    PixBufSetColorFunc(component, colorFunc);
    PixBufSetBlitMode(component, BLIT_COLOR_FUNC);
    PixBufBlit(composeMap, 0, 0, component, NULL);
  } else {
    PixBufClear(composeMap);
    composeMap->fgColor = 1;
    DrawEllipse(composeMap,
                160, 128,
                40 + sin((float)frameNumber / (4.0f * M_PI)) * 40.0f,
                32 + sin((float)frameNumber / (4.0f * M_PI)) * 32.0f);
  }

  UVMapSetOffset(uvmap[0], du, dv);
  UVMapSetOffset(uvmap[1], -du, -dv);
  PROFILE (UVMapCompose1)
    UVMapComposeAndRender(uvmap[0], canvas, composeMap, 0);
  PROFILE (UVMapCompose2)
    UVMapComposeAndRender(uvmap[1], canvas, composeMap, 1);

  c2p1x1_8_c5_bm(canvas->data, GetCurrentBitMap(), WIDTH, HEIGHT, 0, 0);
}

void MainLoop() {
  LoopEventT event = LOOP_CONTINUE;

  SetVBlankCounter(0);

  do {
    int frameNumber = GetVBlankCounter();

    if (event == LOOP_NEXT)
      EffectNum = (EffectNum + 1) % 2;
    if (event == LOOP_PREV)
      EffectNum = (EffectNum - 1) % 2;

    RenderChunky(frameNumber);
    RenderFrameNumber(frameNumber);
    RenderFramesPerSecond(frameNumber);

    DisplaySwap();
  } while ((event = ReadLoopEvent()) != LOOP_EXIT);
}
