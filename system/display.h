#ifndef __SYSTEM_DISPLAY_H__
#define __SYSTEM_DISPLAY_H__

#include <graphics/gfx.h>
#include <graphics/view.h>

#include "gfx/palette.h"

typedef struct BitMap BitMapT;
typedef struct DBufInfo DBufInfoT;
typedef struct View ViewT;
typedef struct ViewPort ViewPortT;

ViewT *NewView();
void DeleteView(ViewT *view);
void ApplyView(ViewT *view, ViewPortT *viewPort);
void SaveOrigView();
void RestoreOrigView();

ViewPortT *NewViewPort(int width, int height, int depth);
void DeleteViewPort(ViewPortT *viewPort);
void ConfigureViewPort(ViewPortT *viewPort);
void LoadPalette(ViewPortT *viewPort, PaletteT *palette);
void SetColor(ViewPortT *viewPort, size_t num, uint8_t r, uint8_t g, uint8_t b);

DBufInfoT *NewDBufInfo(ViewPortT *viewPort, int width, int height, int depth);
void DeleteDBufInfo(DBufInfoT *dbufInfo);

typedef struct DBufRaster {
  ViewPortT *ViewPort;
  DBufInfoT *DBufInfo;
  BitMapT *BitMap;

  LONG CurrentBitMap;
  BOOL SafeToWrite;
  BOOL SafeToSwap;
} DBufRasterT;

DBufRasterT *NewDBufRaster(int width, int height, int depth);
void DeleteDBufRaster(DBufRasterT *raster);
void WaitForSafeToWrite(DBufRasterT *raster);
void WaitForSafeToSwap(DBufRasterT *raster);
void DBufRasterSwap(DBufRasterT *raster);

#endif
