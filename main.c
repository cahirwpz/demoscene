#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/graphics.h>
#include <clib/alib_stdio_protos.h>

#include <inline/exec_protos.h>
#include <inline/dos_protos.h>
#include <inline/graphics_protos.h>

#include <graphics/videocontrol.h>
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

struct TagItem vcTags[] = {
  { VTAG_ATTACH_CM_SET, NULL },
  { VTAG_BORDERBLANK_SET, TRUE },
  { VTAG_BORDERSPRITE_SET, TRUE },
  { VTAG_SPRITERESN_SET, SPRITERESN_ECS },
  { VTAG_END_CM, NULL }
};

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

void Render(struct BitMap *bm, struct ViewPort *vp) {
  APTR modMusic = ReadFileSimple("data/tempest-acidjazzed_evening.p61", MEMF_CHIP);
  UBYTE *txtData = ReadFileSimple("data/texture-01.raw", MEMF_PUBLIC);
  UBYTE *txtPal = ReadFileSimple("data/texture-01.pal", MEMF_PUBLIC);
  UBYTE *chunky = NEW_A(UBYTE, WIDTH * HEIGHT);
  struct DistortionMap *tunnel = NewDistortionMap(WIDTH, HEIGHT);

  if (txtData && txtPal && chunky && tunnel) {
    ViewPortLoadPalette(vp, (UBYTE *)txtPal, 0, HEIGHT);

    GenerateTunnel(tunnel, 8192, WIDTH/2, HEIGHT/2);

    P61_Init(modMusic, NULL, NULL);
    P61_ControlBlock.Play = 1;

    SetVBlankCounter(0);

    while (GetVBlankCounter() < 500) {
      int offset = GetVBlankCounter();

      RenderDistortion(chunky, tunnel, txtData, 0, offset / 10);
      c2p1x1_8_c5_bm(chunky, bm, WIDTH, HEIGHT, 0, 0);

      RenderFrameNumber(bm);
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
  struct View *oldView = GfxBase->ActiView;

  struct View *view = NewView();
  struct BitMap *bm = NewBitMap(WIDTH, HEIGHT, DEPTH);
  struct ColorMap *cm = GetColorMap(1<<DEPTH);
  struct ViewPort *vp = NewViewPort(cm, bm, WIDTH, HEIGHT);

  view->ViewPort = vp;

  vcTags[0].ti_Data = (ULONG)vp;
  VideoControl(cm, vcTags);

  MakeVPort(view, vp);
  MrgCop(view);
  LoadView(view);

  int i;

  for (i=0; i<8; i++)
    FreeSprite(i);

  Render(bm, vp);

  LoadView(oldView);
  WaitTOF();

  DeleteViewPort(vp);
  FreeColorMap(cm);
  DeleteBitMap(bm, WIDTH, HEIGHT, DEPTH);
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
