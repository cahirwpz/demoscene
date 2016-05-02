#include "std/debug.h"
#include "std/memory.h"

#include "gfx/blit.h"
#include "gfx/palette.h"
#include "gfx/png.h"
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

static PixBufT *canvas;
static PixBufT *flare;
static PixBufT *origU;
static PixBufT *origV;
static UVMapT *uvmap;
static PixBufT *texture;
static PaletteT *texturePal;

void AcquireResources() {
  LoadPngImage(&texture, &texturePal, "data/texture-01.png");
}

void ReleaseResources() {
}

bool SetupDisplay() {
  return InitDisplay(WIDTH, HEIGHT, DEPTH);
}

void SetupEffect() {
  float lightRadius = 1.0f;
  int i;

  uvmap = NewUVMap(WIDTH, HEIGHT, UV_FAST, 256, 256);
  canvas = NewPixBuf(PIXBUF_CLUT, WIDTH, HEIGHT);
  origU = NewPixBuf(PIXBUF_GRAY, WIDTH, HEIGHT);
  origV = NewPixBuf(PIXBUF_GRAY, WIDTH, HEIGHT);
  flare = NewPixBuf(PIXBUF_GRAY, 64, 64);

  UVMapGenerateTunnel(uvmap, 32.0f, 3, 4.0 / 3.0, 0.5, 0.5, NULL);
  UVMapSetTexture(uvmap, texture);

  LoadPalette(texturePal);
  PixBufClear(canvas);
  GeneratePixels(flare, (GenPixelFuncT)LightLinearFalloff, &lightRadius);

  for (i = 0; i < flare->width * flare->height; i++)
    flare->data[i] /= 4;

  PixBufBlit(origU, 0, 0,
             NewPixBufWrapper(WIDTH, HEIGHT, uvmap->map.fast.u), NULL);
  PixBufBlit(origV, 0, 0,
             NewPixBufWrapper(WIDTH, HEIGHT, uvmap->map.fast.v), NULL);
}

void TearDownEffect() {
}

void RenderChunky(int frameNumber) {
  PixBufT *umap;

  int du = frameNumber;
  int dv = 2 * frameNumber;
  int i;

  umap = NewPixBufWrapper(WIDTH, HEIGHT, uvmap->map.fast.u);

  UVMapSetOffset(uvmap, du, dv);
  PixBufBlit(umap, 0, 0, origU, NULL);

  for (i = 0; i < 8; i++) {
    PixBufSetBlitMode(flare, BLIT_ADDITIVE);
    PixBufBlit(umap, 
               (int)(128.0f + sin(-frameNumber * M_PI / 45.0f + M_PI * i / 4) * 80.0f), 
               (int)(96.0f + cos(-frameNumber * M_PI / 45.0f + M_PI * i / 4) * 48.0f),
               flare, NULL);
  }

  UVMapRender(uvmap, canvas);

  c2p1x1_8_c5_bm(canvas->data, GetCurrentBitMap(), WIDTH, HEIGHT, 0, 0);

  MemUnref(umap);
}

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
