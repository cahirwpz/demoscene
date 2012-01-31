#include "p61/p61.h"
#include "system/c2p.h"
#include "system/display.h"
#include "system/memory.h"
#include "system/resource.h"
#include "system/vblank.h"

#include "frame_tools.h"
#include "distortion.h"

const int WIDTH = 320;
const int HEIGHT = 256;
const int DEPTH = 8;

static struct DBufRaster *Raster;
static struct DistortionMap *TunnelMap;
static UBYTE *Texture;

/*
 * Set up display function.
 */
struct ViewPort *SetupDisplay() {
  if ((Raster = NewDBufRaster(WIDTH, HEIGHT, DEPTH))) {
    ConfigureViewPort(Raster->ViewPort);
    LoadPalette(Raster->ViewPort, (UBYTE *)GetResource("palette"), 0, 256);

    return Raster->ViewPort;
  }

  return NULL;
}

/*
 * Tear down display function.
 */
void TearDownDisplay() {
  DeleteDBufRaster(Raster);
}

/*
 * Set up effect function.
 */
void SetupEffect() {
  TunnelMap = GetResource("tunnel_map");
  Texture = GetResource("texture");

  P61_Init(GetResource("module"), NULL, NULL);
  P61_ControlBlock.Play = 1;
}

/*
 * Tear down effect function.
 */
void TearDownEffect() {
  P61_End();
}

/*
 * Rendering functions.
 */
void RenderTunnel(int frameNumber, struct DBufRaster *raster) {
  RenderDistortion(&raster->Canvas->bitmap->data, TunnelMap, Texture, 0, frameNumber);
}

void RenderChunky(int frameNumber, struct DBufRaster *raster) {
  c2p1x1_8_c5_bm(&raster->Canvas->bitmap->data, raster->BitMap, WIDTH, HEIGHT, 0, 0);
}

/*
 * Main loop.
 */
void MainLoop() {
  struct DBufRaster *raster = Raster;

  SetVBlankCounter(0);

  while (GetVBlankCounter() < 500) {
    WaitForSafeToWrite(raster);

    int frameNumber = GetVBlankCounter();

    RenderTunnel(frameNumber, raster);
    RenderChunky(frameNumber, raster);
    RenderFrameNumber(frameNumber, raster);

    WaitForSafeToSwap(raster);
    DBufRasterSwap(raster);
  }
}
