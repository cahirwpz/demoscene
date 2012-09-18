#include <proto/graphics.h>

#include <graphics/videocontrol.h>
#include <graphics/gfxbase.h>

#include "std/debug.h"
#include "std/memory.h"
#include "system/display.h"

typedef struct DBufInfo DBufInfoT;
typedef struct View ViewT;
typedef struct ViewPort ViewPortT;

typedef struct ScreenPalette {
  UWORD Count;
  UWORD Start;
  ULONG Colors[256*3+1];
} ScreenPaletteT;

/*
 * ViewPort handling functions.
 */

static void DeleteViewPort(ViewPortT *viewPort) {
  FreeVPortCopLists(viewPort);
  FreeColorMap(viewPort->ColorMap);
  MemUnref(viewPort->RasInfo);
}

TYPEDECL(ViewPortT, (FreeFuncT)DeleteViewPort);

static ViewPortT *NewViewPort(int width, int height, int depth) {
  ViewPortT *viewPort = NewInstance(ViewPortT);

  InitVPort(viewPort);

  viewPort->DWidth = width;
  viewPort->DHeight = height;
  viewPort->RasInfo = NewRecord(struct RasInfo);
  viewPort->ColorMap = GetColorMap(1 << depth);

  if (viewPort->RasInfo && viewPort->ColorMap)
    return viewPort;

  MemUnref(viewPort);

  return NULL;
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

/*
 * DBufRaster handling functions.
 */

typedef struct DBufRaster {
  ViewPortT *ViewPort;
  DBufInfoT *DBufInfo;
  BitMapT *BitMap;
  RastPortT *RastPort;
  ScreenPaletteT *Palette;

  LONG CurrentBitMap;
  BOOL SafeToWrite;
  BOOL SafeToSwap;
} DBufRasterT;

static void DeleteDBufRaster(DBufRasterT *raster) {
  {
    DBufInfoT *dbufInfo = raster->DBufInfo;

    FreeBitMap(dbufInfo->dbi_UserData1);
    FreeBitMap(dbufInfo->dbi_UserData2);
    DeleteMsgPort(dbufInfo->dbi_SafeMessage.mn_ReplyPort);
    DeleteMsgPort(dbufInfo->dbi_DispMessage.mn_ReplyPort);
    FreeDBufInfo(dbufInfo);
  }

  MemUnref(raster->Palette);
  MemUnref(raster->RastPort);
  MemUnref(raster->ViewPort);
}

TYPEDECL(DBufRasterT, (FreeFuncT)DeleteDBufRaster);

DBufRasterT *NewDBufRaster(int width, int height, int depth) {
  DBufRasterT *raster = NewInstance(DBufRasterT);

  raster->ViewPort = NewViewPort(width, height, depth);
  raster->SafeToSwap = TRUE;
  raster->SafeToWrite = TRUE;
  raster->CurrentBitMap = 1;

  {
    int i, j;

    raster->Palette = NewRecord(ScreenPaletteT);
    raster->Palette->Count = 256;

    for (i = 0, j = 0; i < 256; i++) {
      raster->Palette->Colors[j++] = i << 24;
      raster->Palette->Colors[j++] = i << 24;
      raster->Palette->Colors[j++] = i << 24;
    }

    LoadRGB32(raster->ViewPort, (ULONG *)raster->Palette);
  }

  {
    DBufInfoT *dbufInfo;

    if (!(dbufInfo = AllocDBufInfo(raster->ViewPort)))
      PANIC("AllocDBufInfo(..) failed!");

    if (!(dbufInfo->dbi_UserData1 = (APTR)
          AllocBitMap(width, height, depth, BMF_DISPLAYABLE|BMF_CLEAR, NULL)))
      PANIC("AllocBitMap(%d, %d, %d, ...) failed!", width, height, depth);

    if (!(dbufInfo->dbi_UserData2 = (APTR)
          AllocBitMap(width, height, depth, BMF_DISPLAYABLE|BMF_CLEAR, NULL)))
      PANIC("AllocBitMap(%d, %d, %d, ...) failed!", width, height, depth);

    if (!(dbufInfo->dbi_SafeMessage.mn_ReplyPort = CreateMsgPort()))
      PANIC("CreateMsgPort() failed!");

    if (!(dbufInfo->dbi_DispMessage.mn_ReplyPort = CreateMsgPort()))
      PANIC("CreateMsgPort() failed!");

    raster->ViewPort->RasInfo->BitMap = (BitMapT *)dbufInfo->dbi_UserData1;
    raster->BitMap = (BitMapT *)dbufInfo->dbi_UserData2;
    raster->DBufInfo = dbufInfo;
  }

  raster->RastPort = NewRecord(RastPortT);
  InitRastPort(raster->RastPort);
  raster->RastPort->BitMap = raster->BitMap;

  return raster;
}

static void WaitForSafeToWrite(DBufRasterT *raster) {
  if (!raster->SafeToWrite) {
    struct MsgPort *SafeMsgPort = raster->DBufInfo->dbi_SafeMessage.mn_ReplyPort;

    while (!GetMsg(SafeMsgPort))
      Wait(1L << SafeMsgPort->mp_SigBit);

    raster->SafeToWrite = TRUE;
  }
}

static void WaitForSafeToSwap(DBufRasterT *raster) {
  if (!raster->SafeToSwap) {
    struct MsgPort *DispMsgPort = raster->DBufInfo->dbi_DispMessage.mn_ReplyPort;

    while (!GetMsg(DispMsgPort))
      Wait(1L << DispMsgPort->mp_SigBit);

    raster->SafeToSwap = TRUE;
  }
}

static void DBufRasterSwap(DBufRasterT *raster) {
  ChangeVPBitMap(raster->ViewPort, raster->BitMap, raster->DBufInfo);

  raster->BitMap = (BitMapT *)(raster->CurrentBitMap ?
                               raster->DBufInfo->dbi_UserData2 :
                               raster->DBufInfo->dbi_UserData1);
  raster->RastPort->BitMap = raster->BitMap;
  raster->SafeToSwap = FALSE;
  raster->SafeToWrite = FALSE;
  raster->CurrentBitMap ^= 1;
}

/*
 * Display interface implementation.
 */

static DBufRasterT *TheRaster = NULL;
static ViewT *TheView = NULL;
static ViewT *OrigView = NULL;

void LoadPalette(PaletteT *palette) {
  ScreenPaletteT *ThePalette = TheRaster->Palette;
  size_t j = 0;

  ThePalette->Start = palette->start;
  ThePalette->Count = 0;

  while (palette) {
    size_t i;

    for (i = 0; i < palette->count; i++) {
      ThePalette->Colors[j++] = palette->colors[i].r << 24;
      ThePalette->Colors[j++] = palette->colors[i].g << 24;
      ThePalette->Colors[j++] = palette->colors[i].b << 24;
    }

    ThePalette->Count += palette->count;
 
    palette = palette->next;
  }

  ThePalette->Colors[j] = 0;

  LoadRGB32(TheRaster->ViewPort, (ULONG *)ThePalette);
}

bool InitDisplay(int width, int height, int depth) {
  if (!TheRaster) {
    if ((TheRaster = NewDBufRaster(width, height, depth))) {
      /* Save old view. */
      OrigView = GfxBase->ActiView;

      /* Create new view. */
      TheView = NewRecord(ViewT);
      InitView(TheView);

      /* Attach view port. */
      TheView->ViewPort = TheRaster->ViewPort;
      ConfigureViewPort(TheRaster->ViewPort);
      MakeVPort(TheView, TheRaster->ViewPort);

      /* Load new view. */
      MrgCop(TheView);
      LoadView(TheView);

      /* Clean-up Intuition sprites */
      {
        int i;

        for (i = 0; i < 8; i++)
          FreeSprite(i);
      }

      return TRUE;
    }
  }

  return FALSE;
}

void KillDisplay() {
  if (TheRaster) {
    /* Switch to old view. */
    LoadView(OrigView);
    WaitTOF();

    /* Free copper lists. */
    FreeCprList(TheView->LOFCprList);                                                                                                                                                  
    if (TheView->SHFCprList)
      FreeCprList(TheView->SHFCprList);

    MemUnref(TheView);
    MemUnref(TheRaster);

    TheRaster = NULL;
    TheView = NULL;
    OrigView = NULL;
  }
}

void DisplaySwap() {
  WaitForSafeToSwap(TheRaster);
  DBufRasterSwap(TheRaster);
  WaitForSafeToWrite(TheRaster);
}

BitMapT *GetCurrentBitMap() {
  return TheRaster->BitMap;
}

RastPortT *GetCurrentRastPort() {
  return TheRaster->RastPort;
}
