#include "std/debug.h"
#include "std/memory.h"
#include "std/resource.h"

#include "gfx/blit.h"
#include "gfx/colorfunc.h"
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

/*
 * Set up resources.
 */
void AddInitialResources() {
  ResAddPngImage("Texture", "TexturePal", "data/texture-shades.png");
  ResAddPngImage("ColorMap", NULL, "data/texture-shades-map.png");
  ResAdd("Map", NewUVMap(WIDTH, HEIGHT, UV_FAST, 256, 256));
  ResAdd("Shades", NewPixBuf(PIXBUF_GRAY, WIDTH, HEIGHT));
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
  UVMapT *uvmap = R_("Map");
  PixBufT *shades = R_("Shades");

  LoadPalette(R_("TexturePal"));

  UVMapGenerate4(uvmap);
  UVMapSetTexture(uvmap, R_("Texture"));
  uvmap->lightMap = shades;
  PixBufSetColorMap(shades, R_("ColorMap"));

  ResAdd("Component", NewPixBufWrapper(WIDTH, HEIGHT, uvmap->map.fast.u));

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
void RenderEffect(int frameNumber) {
  PixBufT *canvas = R_("Canvas");
  UVMapT *uvmap = R_("Map");
  PixBufT *comp = R_("Component");
  PixBufT *shades = R_("Shades");
  uint8_t *cfunc = R_("ColFunc");

  int du = 2 * frameNumber;
  int dv = 4 * frameNumber;

  {
    int i;

    for (i = 0; i < 256; i++) {
      uint8_t v = i + du;

      if (v >= 128)
        cfunc[i] = ~v * 2;
      else 
        cfunc[i] = v * 2;
    }
  }

  PixBufSetBlitMode(comp, BLIT_COLOR_FUNC);
  PixBufSetColorFunc(comp, cfunc);
  PROFILE (PixBufBlitWithColorFunc)
    PixBufBlit(shades, 0, 0, comp, NULL);

  UVMapSetOffset(uvmap, du, dv);
  PROFILE (UVMapRenderFast)
    UVMapRender(uvmap, canvas);

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

    RenderEffect(frameNumber);
    RenderFrameNumber(frameNumber);
    RenderFramesPerSecond(frameNumber);

    DisplaySwap();
  } while ((event = ReadLoopEvent()) != LOOP_EXIT);
}
