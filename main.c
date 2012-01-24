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

void Render(struct DBufInfo *dbi, struct ViewPort *vp) {
  APTR modMusic = ReadFileSimple("data/tempest-acidjazzed_evening.p61", MEMF_CHIP);
  UBYTE *txtData = ReadFileSimple("data/texture-01.raw", MEMF_PUBLIC);
  UBYTE *txtPal = ReadFileSimple("data/texture-01.pal", MEMF_PUBLIC);
  UBYTE *chunky = NEW_A(UBYTE, WIDTH * HEIGHT);
  struct DistortionMap *tunnel = NewDistortionMap(WIDTH, HEIGHT);

  BOOL SafeToChange = TRUE;
  BOOL SafeToWrite = TRUE;
  int CurrentBuffer = 1;

  struct BitMap *bm = (struct BitMap *)dbi->dbi_UserData2;

  dbi->dbi_SafeMessage.mn_ReplyPort = CreateMsgPort();
  dbi->dbi_DispMessage.mn_ReplyPort = CreateMsgPort();

  if (txtData && txtPal && chunky && tunnel) {
    ViewPortLoadPalette(vp, (UBYTE *)txtPal, 0, HEIGHT);

    GenerateTunnel(tunnel, 8192, WIDTH/2, HEIGHT/2);

    P61_Init(modMusic, NULL, NULL);
    P61_ControlBlock.Play = 1;

    SetVBlankCounter(0);

    while (GetVBlankCounter() < 500) {
      if (!SafeToWrite) {
        struct MsgPort *SafeMsgPort = dbi->dbi_SafeMessage.mn_ReplyPort;

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
        struct MsgPort *DispMsgPort = dbi->dbi_DispMessage.mn_ReplyPort;

        while (!GetMsg(DispMsgPort))
          Wait(1L << DispMsgPort->mp_SigBit);

        SafeToChange = TRUE;
      }

      ChangeVPBitMap(vp, bm, dbi);

      bm = (struct BitMap *)(CurrentBuffer ? dbi->dbi_UserData2 : dbi->dbi_UserData1);

      SafeToChange = FALSE;
      SafeToWrite = FALSE;
      CurrentBuffer ^= 1;
    }

    P61_End();
  }

  DeleteMsgPort(dbi->dbi_SafeMessage.mn_ReplyPort);
  DeleteMsgPort(dbi->dbi_DispMessage.mn_ReplyPort);

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
  struct BitMap *bm1 = NewBitMap(WIDTH, HEIGHT, DEPTH);
  struct BitMap *bm2 = NewBitMap(WIDTH, HEIGHT, DEPTH);
  struct ColorMap *cm = GetColorMap(1<<DEPTH);
  struct ViewPort *vp = NewViewPort(cm, bm1, WIDTH, HEIGHT);
  struct DBufInfo *dbi = AllocDBufInfo(vp);

  dbi->dbi_UserData1 = (APTR)bm1;
  dbi->dbi_UserData2 = (APTR)bm2;

  view->ViewPort = vp;

  vcTags[0].ti_Data = (ULONG)vp;
  VideoControl(cm, vcTags);

  MakeVPort(view, vp);
  MrgCop(view);
  LoadView(view);

  int i;

  for (i=0; i<8; i++)
    FreeSprite(i);

  Render(dbi, vp);

  LoadView(oldView);
  WaitTOF();

  FreeDBufInfo(dbi);
  DeleteViewPort(vp);
  FreeColorMap(cm);
  DeleteBitMap(bm2, WIDTH, HEIGHT, DEPTH);
  DeleteBitMap(bm1, WIDTH, HEIGHT, DEPTH);
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
