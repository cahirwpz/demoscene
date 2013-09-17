#include "std/debug.h"
#include "std/memory.h"
#include "std/resource.h"

#include "gfx/blit.h"
#include "gfx/palette.h"
#include "tools/frame.h"
#include "tools/loopevent.h"
#include "txtgen/procedural.h"

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
  ResAdd("Texture", NewPixBufFromFile("data/texture-01.8"));
  ResAdd("TexturePal", NewPaletteFromFile("data/texture-01.pal"));
  ResAdd("Map", NewUVMap(WIDTH, HEIGHT, UV_NORMAL, 256, 256));
  ResAdd("Canvas", NewPixBuf(PIXBUF_CLUT, WIDTH, HEIGHT));
  ResAdd("OrigU", NewPixBuf(PIXBUF_GRAY, WIDTH, HEIGHT));
  ResAdd("OrigV", NewPixBuf(PIXBUF_GRAY, WIDTH, HEIGHT));
  ResAdd("Flare", NewPixBuf(PIXBUF_GRAY, 64, 64));

  UVMapGenerateTunnel(R_("Map"), 32.0f, 3, 4.0 / 3.0, 0.5, 0.5, NULL);
  UVMapSetTexture(R_("Map"), R_("Texture"));
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
  PixBufT *flare = R_("Flare");
  UVMapT *uvmap = R_("Map");
  float lightRadius = 1.0f;
  int i;

  LoadPalette(R_("TexturePal"));
  PixBufClear(R_("Canvas"));
  GeneratePixels(flare, (GenPixelFuncT)LightLinearFalloff, &lightRadius);

  for (i = 0; i < flare->width * flare->height; i++)
    flare->data[i] /= 4;

  PixBufBlit(R_("OrigU"), 0, 0,
             NewPixBufWrapper(WIDTH, HEIGHT, uvmap->map.normal.u), NULL);
  PixBufBlit(R_("OrigV"), 0, 0,
             NewPixBufWrapper(WIDTH, HEIGHT, uvmap->map.normal.v), NULL);
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
  PixBufT *flare = R_("Flare");
  PixBufT *origU = R_("OrigU");
  PixBufT *umap;

  int du = frameNumber;
  int dv = 2 * frameNumber;
  int i;

  umap = NewPixBufWrapper(WIDTH, HEIGHT, uvmap->map.normal.u);

  UVMapSetOffset(R_("Map"), du, dv);
  PixBufBlit(umap, 0, 0, origU, NULL);

  for (i = 0; i < 8; i++) {
    PixBufSetBlitMode(flare, BLIT_ADDITIVE);
    PixBufBlit(umap, 
               (int)(128.0f + sin(-frameNumber * M_PI / 45.0f + M_PI * i / 4) * 80.0f), 
               (int)(96.0f + cos(-frameNumber * M_PI / 45.0f + M_PI * i / 4) * 48.0f),
               flare, NULL);
  }

  UVMapRender(R_("Map"), canvas);

  c2p1x1_8_c5_bm(canvas->data, GetCurrentBitMap(), WIDTH, HEIGHT, 0, 0);

  MemUnref(umap);
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
