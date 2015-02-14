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

  if (width > 360)
    viewPort->Modes |= HIRES;
  if (height > 288)
    viewPort->Modes |= LACE;

  if (viewPort->RasInfo && viewPort->ColorMap)
    return viewPort;

  MemUnref(viewPort);

  return NULL;
}

static void ConfigureView(ViewT *view, int width, int height) {
  /* Center view */
  if (height < 256)
    view->DyOffset += (256 - height) / 2;
  if (height > 288 && height < 512)
    view->DyOffset += (512 - height) / 4;
  if (width < 320)
    view->DxOffset += (320 - width) / 2;
}

static struct TagItem VideoCtrlTags[] = {
  { VTAG_BORDERBLANK_SET, TRUE },
  { VTAG_BORDERSPRITE_SET, TRUE },
  { VTAG_SPRITERESN_SET, SPRITERESN_ECS },
  { VTAG_END_CM, 0L }
};

static void ConfigureViewPort(ViewPortT *viewPort) {
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
  BOOL SafeToSwap;
} DBufRasterT;

static void DeleteDBufRaster(DBufRasterT *raster) {
  DBufInfoT *dbufInfo = raster->DBufInfo;

  FreeBitMap(dbufInfo->dbi_UserData1);
  FreeBitMap(dbufInfo->dbi_UserData2);
  (void)GetMsg(dbufInfo->dbi_DispMessage.mn_ReplyPort);
  DeleteMsgPort(dbufInfo->dbi_DispMessage.mn_ReplyPort);
  FreeDBufInfo(dbufInfo);

  MemUnref(raster->Palette);
  MemUnref(raster->RastPort);
  MemUnref(raster->ViewPort);
}

TYPEDECL(DBufRasterT, (FreeFuncT)DeleteDBufRaster);

DBufRasterT *NewDBufRaster(int width, int height, int depth) {
  DBufRasterT *raster = NewInstance(DBufRasterT);

  raster->ViewPort = NewViewPort(width, height, depth);
  raster->SafeToSwap = TRUE;
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

/* Clean-up Intuition sprites */
static void ClearSprites() {
  int i;

  for (i = 0; i < 8; i++)
    FreeSprite(i);
}

bool InitDisplay(int width, int height, int depth) {
  if (!TheRaster) {
    if ((TheRaster = NewDBufRaster(width, height, depth))) {
      /* Save old view. */
      OrigView = GfxBase->ActiView;

      /* Create new view. */
      TheView = NewRecord(ViewT);
      InitView(TheView);
      ConfigureView(TheView, width, height);

      /* Attach view port. */
      TheView->ViewPort = TheRaster->ViewPort;
      ConfigureViewPort(TheRaster->ViewPort);
      MakeVPort(TheView, TheRaster->ViewPort);

      /* Load new view. */
      MrgCop(TheView);
      LoadView(TheView);
      ClearSprites();

      return true;
    }
  }

  return false;
}

bool ChangeDisplay(int width, int height, int depth) {
  if (TheRaster) {
    ViewT *OldView = TheView;
    DBufRasterT *OldRaster = TheRaster;

    if ((TheRaster = NewDBufRaster(width, height, depth))) {
      /* Create new view. */
      TheView = NewRecord(ViewT);
      InitView(TheView);
      ConfigureView(TheView, width, height);

      /* Attach view port. */
      TheView->ViewPort = TheRaster->ViewPort;
      ConfigureViewPort(TheRaster->ViewPort);
      MakeVPort(TheView, TheRaster->ViewPort);

      /* Load new view. */
      MrgCop(TheView);
      LoadView(TheView);
      ClearSprites();

      FreeCprList(OldView->LOFCprList);                                                                                                                                                  
      if (OldView->SHFCprList)
        FreeCprList(OldView->SHFCprList);

      MemUnref(OldView);
      MemUnref(OldRaster);

      return true;
    }
  }

  return false;
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
}

BitMapT *GetCurrentBitMap() {
  return TheRaster->BitMap;
}

RastPortT *GetCurrentRastPort() {
  return TheRaster->RastPort;
}
