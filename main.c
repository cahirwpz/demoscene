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

void Render(struct DBufRaster *raster) {
  APTR modMusic = ReadFileSimple("data/tempest-acidjazzed_evening.p61", MEMF_CHIP);
  UBYTE *txtData = ReadFileSimple("data/texture-01.raw", MEMF_PUBLIC);
  UBYTE *txtPal = ReadFileSimple("data/texture-01.pal", MEMF_PUBLIC);
  UBYTE *chunky = NEW_A(UBYTE, WIDTH * HEIGHT);
  struct DistortionMap *tunnel = NewDistortionMap(WIDTH, HEIGHT);

  BOOL SafeToChange = TRUE;
  BOOL SafeToWrite = TRUE;
  int CurrentBuffer = 1;

  struct BitMap *bm = (struct BitMap *)raster->DBufInfo->dbi_UserData2;

  raster->DBufInfo->dbi_SafeMessage.mn_ReplyPort = CreateMsgPort();
  raster->DBufInfo->dbi_DispMessage.mn_ReplyPort = CreateMsgPort();

  if (txtData && txtPal && chunky && tunnel) {
    ViewPortLoadPalette(raster->ViewPort, (UBYTE *)txtPal, 0, HEIGHT);

    GenerateTunnel(tunnel, 8192, WIDTH/2, HEIGHT/2);

    P61_Init(modMusic, NULL, NULL);
    P61_ControlBlock.Play = 1;

    SetVBlankCounter(0);

    while (GetVBlankCounter() < 500) {
      if (!SafeToWrite) {
        struct MsgPort *SafeMsgPort = raster->DBufInfo->dbi_SafeMessage.mn_ReplyPort;

        while (!GetMsg(SafeMsgPort))
          Wait(1L << SafeMsgPort->mp_SigBit);

        SafeToWrite = TRUE;
      }

      {
        int offset = GetVBlankCounter();

        RenderDistortion(chunky, tunnel, txtData, 0, offset);
        c2p1x1_8_c5_bm(chunky, bm, WIDTH, HEIGHT, 0, 0);
        RenderFrameNumber(bm);
      }

      if (!SafeToChange) {
        struct MsgPort *DispMsgPort = raster->DBufInfo->dbi_DispMessage.mn_ReplyPort;

        while (!GetMsg(DispMsgPort))
          Wait(1L << DispMsgPort->mp_SigBit);

        SafeToChange = TRUE;
      }

      ChangeVPBitMap(raster->ViewPort, bm, raster->DBufInfo);

      bm = (struct BitMap *)(CurrentBuffer ? raster->DBufInfo->dbi_UserData2 : raster->DBufInfo->dbi_UserData1);

      SafeToChange = FALSE;
      SafeToWrite = FALSE;
      CurrentBuffer ^= 1;
    }

    P61_End();
  }

  DeleteMsgPort(raster->DBufInfo->dbi_SafeMessage.mn_ReplyPort);
  DeleteMsgPort(raster->DBufInfo->dbi_DispMessage.mn_ReplyPort);

  if (tunnel)
    DeleteDistortionMap(tunnel);

  DELETE(modMusic);
  DELETE(chunky);
  DELETE(txtData);
  DELETE(txtPal);
}

void start() {
  struct DBufRaster *raster = NewDBufRaster(WIDTH, HEIGHT, DEPTH);

  vcTags[0].ti_Data = (ULONG)raster->ViewPort;
  VideoControl(raster->ViewPort->ColorMap, vcTags);

  struct View *oldView = GfxBase->ActiView;
  struct View *view = NewView();

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
