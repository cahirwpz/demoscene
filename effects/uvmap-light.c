#include "std/debug.h"
#include "std/memory.h"
#include "std/resource.h"

#include "gfx/blit.h"
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
  ResAdd("Texture", NewPixBufFromFile("data/texture-shades.8"));
  ResAdd("TexturePal", NewPaletteFromFile("data/texture-shades.pal"));
  ResAdd("ColorMap", NewPixBufFromFile("data/texture-shades-map.8"));
  ResAdd("Map", NewUVMap(WIDTH, HEIGHT, UV_OPTIMIZED, 256, 256));
  ResAdd("Shades", NewPixBuf(PIXBUF_GRAY, WIDTH, HEIGHT));
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
  LoadPalette(R_("TexturePal"));

  UVMapGenerate4(R_("Map"));
  UVMapSetTexture(R_("Map"), R_("Texture"));
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
  UVMapT *uvmap = R_("Map");
  PixBufT *shades = R_("Shades");

  int du = 2 * frameNumber;
  int dv = 4 * frameNumber;

  UVMapSetOffset(uvmap, du, dv);
  UVMapRender(uvmap, canvas);

  UVMapSetOffset(uvmap, -du, -dv / 2);
  UVMapLayers(uvmap, UV_EXTRACT_U, shades);

  {
    int n = shades->width * shades->height;
    uint8_t *d = shades->data;

    do {
      uint8_t v = *d;

      if (v >= 128)
        *d++ = ~v * 2;
      else 
        *d++ = v * 2;
    } while (--n);
  }

  PixBufSetColorMap(shades, R_("ColorMap"), 0);
  PixBufSetBlitMode(shades, BLIT_WITH_COLORMAP);

  PixBufBlit(canvas, 0, 0, shades, NULL);

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
