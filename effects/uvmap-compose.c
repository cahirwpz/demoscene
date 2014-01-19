#include "std/debug.h"
#include "std/memory.h"
#include "std/resource.h"

#include "gfx/blit.h"
#include "gfx/colorfunc.h"
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

/*
 * Set up resources.
 */
void AddInitialResources() {
  ResAddPngImage("Texture1", "Texture1Pal", "data/texture-128-01.png");
  ResAddPngImage("Texture2", "Texture2Pal", "data/texture-128-02.png");
  ResAdd("Map1", NewUVMap(WIDTH, HEIGHT, UV_FAST, 256, 256));
  ResAdd("Map2", NewUVMap(WIDTH, HEIGHT, UV_FAST, 256, 256));
  ResAdd("ComposeMap", NewPixBuf(PIXBUF_GRAY, WIDTH, HEIGHT));
  ResAdd("Canvas", NewPixBuf(PIXBUF_CLUT, WIDTH, HEIGHT));
  ResAdd("ColFunc", NewColorFunc());
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
  UVMapT *map1 = R_("Map1");
  UVMapT *map2 = R_("Map2");

  LinkPalettes(R_("Texture1Pal"), R_("Texture2Pal"), NULL);
  LoadPalette(R_("Texture1Pal"));

  PixBufRemap(R_("Texture2"), R_("Texture2Pal"));

  UVMapGenerate3(map1);
  UVMapSetTexture(map1, R_("Texture1"));

  UVMapGenerate4(map2);
  UVMapSetTexture(map2, R_("Texture2"));

  ResAdd("Component", NewPixBufWrapper(WIDTH, HEIGHT, map2->map.fast.v));

  StartProfiling();
}

/*
 * Tear down effect function.
 */
void TearDownEffect() {
  UnlinkPalettes(R_("Texture1Pal"));
  StopProfiling();
}

/*
 * Effect rendering functions.
 */

static int EffectNum = 0;

void RenderChunky(int frameNumber) {
  PixBufT *canvas = R_("Canvas");
  PixBufT *compMap = R_("ComposeMap");
  UVMapT *map1 = R_("Map1");
  UVMapT *map2 = R_("Map2");
  PixBufT *comp = R_("Component");
  int du = 2 * frameNumber;
  int dv = 4 * frameNumber;

  if (EffectNum == 0) {
    uint8_t *cfunc = R_("ColFunc");
    int i;

    for (i = 0; i < 256; i++)
      cfunc[i] = ((128 - ((frameNumber * 2) % 256 + i)) & 0xff) >= 128 ? 1 : 0;

    PixBufSetColorFunc(comp, cfunc);
    PixBufSetBlitMode(comp, BLIT_COLOR_FUNC);
    PixBufBlit(compMap, 0, 0, comp, NULL);
  } else {
    PixBufClear(compMap);
    compMap->fgColor = 1;
    DrawEllipse(compMap,
                160, 128,
                40 + sin((float)frameNumber / (4.0f * M_PI)) * 40.0f,
                32 + sin((float)frameNumber / (4.0f * M_PI)) * 32.0f);
  }

  UVMapSetOffset(map1, du, dv);
  UVMapSetOffset(map2, -du, -dv);
  PROFILE (UVMapCompose1)
    UVMapComposeAndRender(map1, canvas, compMap, 0);
  PROFILE (UVMapCompose2)
    UVMapComposeAndRender(map2, canvas, compMap, 1);

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
      EffectNum = (EffectNum + 1) % 2;
    if (event == LOOP_PREV)
      EffectNum = (EffectNum - 1) % 2;

    RenderChunky(frameNumber);
    RenderFrameNumber(frameNumber);
    RenderFramesPerSecond(frameNumber);

    DisplaySwap();
  } while ((event = ReadLoopEvent()) != LOOP_EXIT);
}
