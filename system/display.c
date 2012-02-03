#include <graphics/videocontrol.h>
#include <proto/graphics.h>
#include <inline/graphics_protos.h>

#include <graphics/gfxbase.h>

#include "display.h"
#include "memory.h"

/*
 * View handling functions.
 */

struct View *NewView() {
  struct View *view = NEW_SZ(struct View);

  if (view) {
    struct ViewExtra *viewExtra = GfxNew(VIEW_EXTRA_TYPE);

    if (viewExtra) {
      InitView(view);
      GfxAssociate(view, viewExtra);

      viewExtra->Monitor = OpenMonitor(NULL, DEFAULT_MONITOR_ID);

      if (viewExtra->Monitor)
        return view;
    }

    DeleteView(view);
  }

  return NULL;
}

void DeleteView(struct View *view) {
  if (view) {
    struct ViewExtra *viewExtra = GfxLookUp(view);

    if (viewExtra) {
      CloseMonitor(viewExtra->Monitor);
      GfxFree(viewExtra);
    }

    FreeCprList(view->LOFCprList);

    if (view->SHFCprList)
      FreeCprList(view->SHFCprList);

    DELETE(view);
  }
}

static struct View *OrigView;

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

void ApplyView(struct View *view, struct ViewPort *viewPort) {
  view->ViewPort = viewPort;

  MakeVPort(view, viewPort);
  MrgCop(view);
  LoadView(view);
}

/*
 * ViewPort handling functions.
 */

struct ViewPort *NewViewPort(int width, int height, int depth) {
  struct ViewPort *viewPort = NEW_SZ(struct ViewPort);

  if (viewPort) {
    InitVPort(viewPort);

    viewPort->DWidth = width;
    viewPort->DHeight = height;
    viewPort->RasInfo = NEW_SZ(struct RasInfo);
    viewPort->ColorMap = GetColorMap(1 << depth);

    if (viewPort->RasInfo && viewPort->ColorMap)
      return viewPort;

    DeleteViewPort(viewPort);
  }

  return NULL;
}

void DeleteViewPort(struct ViewPort *viewPort) {
  if (viewPort) {
    FreeVPortCopLists(viewPort);
    FreeColorMap(viewPort->ColorMap);

    DELETE(viewPort->RasInfo);
    DELETE(viewPort);
  }
}

static struct TagItem VideoCtrlTags[] = {
  { VTAG_ATTACH_CM_SET, NULL },
  { VTAG_BORDERBLANK_SET, TRUE },
  { VTAG_BORDERSPRITE_SET, TRUE },
  { VTAG_SPRITERESN_SET, SPRITERESN_ECS },
  { VTAG_END_CM, NULL }
};

void ConfigureViewPort(struct ViewPort *viewPort) {
  VideoCtrlTags[0].ti_Data = (ULONG)viewPort;
  VideoControl(viewPort->ColorMap, VideoCtrlTags);
}

void LoadPalette(struct ViewPort *viewPort, UBYTE *components, UWORD start,
                 UWORD count) {
  ULONG *palette;

  if ((palette = NEW_A(ULONG, count * 3 + 1))) {
    int i;

    palette[0] = (count << 16) | start;

    for (i = 0; i < count * 3; i++)
      palette[i+1] = (ULONG)components[i] << 24;

    LoadRGB32(viewPort, palette);

    DELETE(palette);
  }
}

/*
 * DBufInfo handling functions.
 */

struct DBufInfo *NewDBufInfo(struct ViewPort *viewPort, int width, int height, int depth) {
  struct DBufInfo *dbufInfo = AllocDBufInfo(viewPort);

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

void DeleteDBufInfo(struct DBufInfo *dbufInfo) {
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

struct DBufRaster *NewDBufRaster(int width, int height, int depth) {
  struct DBufRaster *raster = NEW_SZ(struct DBufRaster);

  if (raster) {
    struct ViewPort *viewPort = NewViewPort(width, height, depth);

    if (viewPort) {
      raster->Canvas = canvas_new(width, height);
      raster->ViewPort = viewPort;
      raster->DBufInfo = NewDBufInfo(viewPort, width, height, depth);
      raster->SafeToSwap = TRUE;
      raster->SafeToWrite = TRUE;
      raster->CurrentBitMap = 1;

      if (raster->Canvas && raster->DBufInfo) {
        viewPort->RasInfo->BitMap = (struct BitMap *)raster->DBufInfo->dbi_UserData1;
        raster->BitMap = (struct BitMap *)raster->DBufInfo->dbi_UserData2;

        return raster;
      }
    }

    DeleteDBufRaster(raster);
  }

  return NULL;
}

void DeleteDBufRaster(struct DBufRaster *raster) {
  canvas_delete(raster->Canvas);
  DeleteDBufInfo(raster->DBufInfo);
  DeleteViewPort(raster->ViewPort);
  DELETE(raster);
}

void WaitForSafeToWrite(struct DBufRaster *raster) {
  if (!raster->SafeToWrite) {
    struct MsgPort *SafeMsgPort = raster->DBufInfo->dbi_SafeMessage.mn_ReplyPort;

    while (!GetMsg(SafeMsgPort))
      Wait(1L << SafeMsgPort->mp_SigBit);

    raster->SafeToWrite = TRUE;
  }
}

void WaitForSafeToSwap(struct DBufRaster *raster) {
  if (!raster->SafeToSwap) {
    struct MsgPort *DispMsgPort = raster->DBufInfo->dbi_DispMessage.mn_ReplyPort;

    while (!GetMsg(DispMsgPort))
      Wait(1L << DispMsgPort->mp_SigBit);

    raster->SafeToSwap = TRUE;
  }
}

void DBufRasterSwap(struct DBufRaster *raster) {
  ChangeVPBitMap(raster->ViewPort, raster->BitMap, raster->DBufInfo);

  raster->BitMap = (struct BitMap *)(raster->CurrentBitMap ?
                                     raster->DBufInfo->dbi_UserData2 :
                                     raster->DBufInfo->dbi_UserData1);
  raster->SafeToSwap = FALSE;
  raster->SafeToWrite = FALSE;
  raster->CurrentBitMap ^= 1;
}

