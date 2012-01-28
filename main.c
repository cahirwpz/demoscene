#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/graphics.h>
#include <clib/alib_stdio_protos.h>

#include <inline/exec_protos.h>
#include <inline/dos_protos.h>
#include <inline/graphics_protos.h>

#include <graphics/gfxbase.h>

#include "input.h"
#include "fileio.h"
#include "display.h"
#include "vblank.h"
#include "c2p.h"
#include "resource.h"

#include "p61/p61.h"
#include "common.h"
#include "distortion.h"

struct DosLibrary *DOSBase;
struct GfxBase *GfxBase;

const int WIDTH = 320;
const int HEIGHT = 256;
const int DEPTH = 8;

void RenderFrameNumber(struct BitMap *bm) {
  struct RastPort rastPort;

  InitRastPort(&rastPort);
  rastPort.BitMap = bm;
  SetDrMd(&rastPort, JAM1);

  int frameNumber = GetVBlankCounter();

  char number[4];

  sprintf(number, "%04d", frameNumber);

  Move(&rastPort, 2, 8);
  Text(&rastPort, number, 4);
}

void MainLoop(struct DBufRaster *raster) {
  LoadPalette(raster->ViewPort, (UBYTE *)GetResource("palette"), 0, 256);

  P61_Init(GetResource("module"), NULL, NULL);
  P61_ControlBlock.Play = 1;

  SetVBlankCounter(0);

  struct DistortionMap *tunnelMap = GetResource("tunnel_map");
  APTR chunky = GetResource("chunky");
  APTR texture = GetResource("texture");

  while (GetVBlankCounter() < 500) {
    WaitForSafeToWrite(raster);

    int offset = GetVBlankCounter();

    RenderDistortion(chunky, tunnelMap, texture, 0, offset);
    c2p1x1_8_c5_bm(chunky, raster->BitMap, WIDTH, HEIGHT, 0, 0);
    RenderFrameNumber(raster->BitMap);

    WaitForSafeToSwap(raster);
    DBufRasterSwap(raster);
  }

  P61_End();
}

void SetupDisplayAndRun() {
  struct DBufRaster *raster = NewDBufRaster(WIDTH, HEIGHT, DEPTH);
  struct View *oldView = GfxBase->ActiView;
  struct View *view = NewView();

  ConfigureViewPort(raster->ViewPort);

  /* Attach ViewPort to View */
  view->ViewPort = raster->ViewPort;
  MakeVPort(view, raster->ViewPort);

  /* Load new View */
  MrgCop(view);
  LoadView(view);

  /* TODO: Prevent intution from grabbing input */
  int i;

  for (i=0; i<8; i++)
    FreeSprite(i);

  MainLoop(raster);

  /* Restore old View */
  LoadView(oldView);
  WaitTOF();

  /* Deinitialize display related structures */
  DeleteDBufRaster(raster);
  DeleteView(view);
}

int main() {
  if ((DOSBase = (struct DosLibrary *)OpenLibrary("dos.library", 40))) {
    if ((GfxBase = (struct GfxBase *)OpenLibrary("graphics.library", 40))) {
      if (ResourcesAlloc() && ResourcesInit()) {
        if (InitEventHandler()) {
          InstallVBlankIntServer();
          SetupDisplayAndRun();
          RemoveVBlankIntServer();
          KillEventHandler();
        }
      }
      ResourcesFree();
      CloseLibrary((struct Library *)GfxBase);
    }
    CloseLibrary((struct Library *)DOSBase);
  }

  return 0;
}
