#include <proto/graphics.h>
#include <inline/graphics_protos.h>

#include <graphics/videocontrol.h>
#include <graphics/gfxbase.h>

#include "display.h"
#include "memory.h"

/*
 * View handling functions.
 */

ViewT *NewView() {
  ViewT *view = NEW_SZ(ViewT);

  if (view)
    InitView(view);

  return view;
}

void DeleteView(ViewT *view) {
  if (view) {
    FreeCprList(view->LOFCprList);                                                                                                                                                  

    if (view->SHFCprList)
      FreeCprList(view->SHFCprList);

    DELETE(view);
  }
}

static ViewT *OrigView;

void SaveOrigView() {
  OrigView = GfxBase->ActiView;

  /* Clean-up Intuition sprites */
  int i;

  for (i = 0; i < 8; i++)
    FreeSprite(i);
}

void RestoreOrigView() {
  LoadView(OrigView);
  WaitTOF();
}

void ApplyView(ViewT *view, ViewPortT *viewPort) {
  view->ViewPort = viewPort;

  MakeVPort(view, viewPort);
  MrgCop(view);
  LoadView(view);
}

/*
 * ViewPort handling functions.
 */

ViewPortT *NewViewPort(int width, int height, int depth) {
  ViewPortT *viewPort = NEW_SZ(ViewPortT);

  if (viewPort) {
    InitVPort(viewPort);

    viewPort->DWidth = width;
    viewPort->DHeight = height;
    viewPort->RasInfo = NEW_SZ(struct RasInfo);
    viewPort->ColorMap = GetColorMap(1 << depth);

    if (viewPort->RasInfo && viewPort->ColorMap) {
      AttachPalExtra(viewPort->ColorMap, viewPort);
      return viewPort;
    }

    DeleteViewPort(viewPort);
  }

  return NULL;
}

void DeleteViewPort(ViewPortT *viewPort) {
  if (viewPort) {
    FreeVPortCopLists(viewPort);
    FreeColorMap(viewPort->ColorMap);

    DELETE(viewPort->RasInfo);
    DELETE(viewPort);
  }
}

static struct TagItem VideoCtrlTags[] = {
  { VTAG_BORDERBLANK_SET, TRUE },
  { VTAG_BORDERSPRITE_SET, TRUE },
  { VTAG_SPRITERESN_SET, SPRITERESN_ECS },
  { VTAG_END_CM, 0L }
};

void ConfigureViewPort(ViewPortT *viewPort) {
  VideoControl(viewPort->ColorMap, VideoCtrlTags);
}

void LoadPalette(ViewPortT *viewPort, UBYTE *components, int start, int count) {
  int i;

  for (i = 0; i < count; i++) {
    int r = *components++;
    int g = *components++;
    int b = *components++;

    SetRGB32CM(viewPort->ColorMap, start + i, r << 24, g << 24, b << 24);
  }
}

/*
 * DBufInfo handling functions.
 */

DBufInfoT *NewDBufInfo(ViewPortT *viewPort, int width, int height, int depth) {
  DBufInfoT *dbufInfo = AllocDBufInfo(viewPort);

  if (dbufInfo) {
    dbufInfo->dbi_UserData1 = (APTR)
      AllocBitMap(width, height, depth, BMF_DISPLAYABLE|BMF_CLEAR, NULL);
    dbufInfo->dbi_UserData2 = (APTR)
      AllocBitMap(width, height, depth, BMF_DISPLAYABLE|BMF_CLEAR, NULL); 

    dbufInfo->dbi_SafeMessage.mn_ReplyPort = CreateMsgPort();
    dbufInfo->dbi_DispMessage.mn_ReplyPort = CreateMsgPort();

    if (dbufInfo->dbi_UserData1 &&
        dbufInfo->dbi_UserData2 && 
        dbufInfo->dbi_SafeMessage.mn_ReplyPort &&
        dbufInfo->dbi_DispMessage.mn_ReplyPort)
      return dbufInfo;

    DeleteDBufInfo(dbufInfo);
  }
  
  return NULL;
}

void DeleteDBufInfo(DBufInfoT *dbufInfo) {
  if (dbufInfo) {
    FreeBitMap(dbufInfo->dbi_UserData1);
    FreeBitMap(dbufInfo->dbi_UserData2);

    DeleteMsgPort(dbufInfo->dbi_SafeMessage.mn_ReplyPort);
    DeleteMsgPort(dbufInfo->dbi_DispMessage.mn_ReplyPort);

    FreeDBufInfo(dbufInfo);
  }
}

/*
 * DBufRaster handling functions.
 */

DBufRasterT *NewDBufRaster(int width, int height, int depth) {
  DBufRasterT *raster = NEW_SZ(DBufRasterT);

  if (raster) {
    ViewPortT *viewPort = NewViewPort(width, height, depth);

    if (viewPort) {
      raster->Canvas = NewCanvas(width, height);
      raster->ViewPort = viewPort;
      raster->DBufInfo = NewDBufInfo(viewPort, width, height, depth);
      raster->SafeToSwap = TRUE;
      raster->SafeToWrite = TRUE;
      raster->CurrentBitMap = 1;

      if (raster->Canvas && raster->DBufInfo) {
        viewPort->RasInfo->BitMap = (BitMapT *)raster->DBufInfo->dbi_UserData1;
        raster->BitMap = (BitMapT *)raster->DBufInfo->dbi_UserData2;

        return raster;
      }
    }

    DeleteDBufRaster(raster);
  }

  return NULL;
}

void DeleteDBufRaster(DBufRasterT *raster) {
  DeleteCanvas(raster->Canvas);
  DeleteDBufInfo(raster->DBufInfo);
  DeleteViewPort(raster->ViewPort);
  DELETE(raster);
}

void WaitForSafeToWrite(DBufRasterT *raster) {
  if (!raster->SafeToWrite) {
    struct MsgPort *SafeMsgPort = raster->DBufInfo->dbi_SafeMessage.mn_ReplyPort;

    while (!GetMsg(SafeMsgPort))
      Wait(1L << SafeMsgPort->mp_SigBit);

    raster->SafeToWrite = TRUE;
  }
}

void WaitForSafeToSwap(DBufRasterT *raster) {
  if (!raster->SafeToSwap) {
    struct MsgPort *DispMsgPort = raster->DBufInfo->dbi_DispMessage.mn_ReplyPort;

    while (!GetMsg(DispMsgPort))
      Wait(1L << DispMsgPort->mp_SigBit);

    raster->SafeToSwap = TRUE;
  }
}

void DBufRasterSwap(DBufRasterT *raster) {
  ChangeVPBitMap(raster->ViewPort, raster->BitMap, raster->DBufInfo);

  raster->BitMap = (BitMapT *)(raster->CurrentBitMap ?
                               raster->DBufInfo->dbi_UserData2 :
                               raster->DBufInfo->dbi_UserData1);
  raster->SafeToSwap = FALSE;
  raster->SafeToWrite = FALSE;
  raster->CurrentBitMap ^= 1;
}
