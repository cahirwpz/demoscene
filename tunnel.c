#include <clib/graphics_protos.h>
#include <inline/graphics_protos.h>
#include <proto/graphics.h>

#include <graphics/gfxbase.h>

#include "p61/p61.h"
#include "system/c2p.h"
#include "system/common.h"
#include "system/display.h"
#include "system/resource.h"
#include "system/vblank.h"

#include "frame_tools.h"
#include "distortion.h"

const int WIDTH = 320;
const int HEIGHT = 256;
const int DEPTH = 8;

struct TunnelData {
  struct DistortionMap *TunnelMap;
  UBYTE *Texture;
} tunnel;

void RenderTunnel(int frameNumber, struct DBufRaster *raster) {
  RenderDistortion(raster->Chunky, tunnel.TunnelMap, tunnel.Texture, 0, frameNumber);
}

void RenderChunky(int frameNumber, struct DBufRaster *raster) {
  c2p1x1_8_c5_bm(raster->Chunky, raster->BitMap, WIDTH, HEIGHT, 0, 0);
}

void MainLoop(struct DBufRaster *raster) {
  LoadPalette(raster->ViewPort, (UBYTE *)GetResource("palette"), 0, 256);

  P61_Init(GetResource("module"), NULL, NULL);
  P61_ControlBlock.Play = 1;

  tunnel.TunnelMap = GetResource("tunnel_map");
  tunnel.Texture = GetResource("texture");

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

  P61_End();
}

void SetupDisplayAndRun() {
  struct DBufRaster *raster = NewDBufRaster(WIDTH, HEIGHT, DEPTH);

  ConfigureViewPort(raster->ViewPort);

  struct View *oldView = GfxBase->ActiView;
  struct View *view = NewView(raster->ViewPort);

  MainLoop(raster);

  /* Restore old View */
  LoadView(oldView);
  WaitTOF();

  /* Deinitialize display related structures */
  DeleteDBufRaster(raster);
  DeleteView(view);
}
