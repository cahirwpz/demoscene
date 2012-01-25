#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/graphics.h>
#include <clib/alib_stdio_protos.h>

#include <inline/exec_protos.h>
#include <inline/dos_protos.h>
#include <inline/graphics_protos.h>

#include <graphics/gfxbase.h>

#include "fileio.h"
#include "display.h"
#include "vblank.h"
#include "c2p.h"

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

void Render(struct DBufRaster *raster) {
  APTR modMusic = ReadFileSimple("data/tempest-acidjazzed_evening.p61", MEMF_CHIP);
  UBYTE *txtData = ReadFileSimple("data/texture-01.raw", MEMF_PUBLIC);
  UBYTE *txtPal = ReadFileSimple("data/texture-01.pal", MEMF_PUBLIC);
  UBYTE *chunky = NEW_A(UBYTE, WIDTH * HEIGHT);
  struct DistortionMap *tunnel = NewDistortionMap(WIDTH, HEIGHT);

  if (txtData && txtPal && chunky && tunnel) {
    ViewPortLoadPalette(raster->ViewPort, (UBYTE *)txtPal, 0, HEIGHT);

    GenerateTunnel(tunnel, 8192, WIDTH/2, HEIGHT/2);

    P61_Init(modMusic, NULL, NULL);
    P61_ControlBlock.Play = 1;

    SetVBlankCounter(0);

    while (GetVBlankCounter() < 500) {
      WaitForSafeToWrite(raster);

      int offset = GetVBlankCounter();

      RenderDistortion(chunky, tunnel, txtData, 0, offset);
      c2p1x1_8_c5_bm(chunky, raster->BitMap, WIDTH, HEIGHT, 0, 0);
      RenderFrameNumber(raster->BitMap);

      WaitForSafeToSwap(raster);
      DBufRasterSwap(raster);
    }

    P61_End();
  }

  if (tunnel)
    DeleteDistortionMap(tunnel);

  DELETE(modMusic);
  DELETE(chunky);
  DELETE(txtData);
  DELETE(txtPal);
}

void start() {
  struct DBufRaster *raster = NewDBufRaster(WIDTH, HEIGHT, DEPTH);
  struct View *oldView = GfxBase->ActiView;
  struct View *view = NewView();

  ConfigureViewPort(raster->ViewPort);

  view->ViewPort = raster->ViewPort;
  MakeVPort(view, raster->ViewPort);

  MrgCop(view);
  LoadView(view);

  int i;

  for (i=0; i<8; i++)
    FreeSprite(i);

  Render(raster);

  LoadView(oldView);
  WaitTOF();

  DeleteDBufRaster(raster);
  DeleteView(view);
}

int main() {
  if ((DOSBase = (struct DosLibrary *)OpenLibrary("dos.library", 40))) {
    if ((GfxBase = (struct GfxBase *)OpenLibrary("graphics.library", 40))) {
      InstallVBlankIntServer();

      start();

      RemoveVBlankIntServer();

      CloseLibrary((struct Library *)GfxBase);
    }
    CloseLibrary((struct Library *)DOSBase);
  }

  return 0;
}
