#include <proto/graphics.h>
#include <inline/graphics_protos.h>

#include "common.h"
#include "display.h"

struct ViewPort *NewViewPort(struct ColorMap *colormap, struct BitMap *bitmap,
                             SHORT width, SHORT height) {
  struct ViewPort *viewPort;
  struct RasInfo *rasInfo;

  if ((viewPort = NEW_SZ(struct ViewPort))) {
    if ((rasInfo = NEW_SZ(struct RasInfo))) {
      rasInfo->BitMap = bitmap;

      InitVPort(viewPort);

      viewPort->ColorMap = colormap;
      viewPort->RasInfo = rasInfo;
      viewPort->DWidth  = width;
      viewPort->DHeight = height;
    } else {
      DELETE(viewPort);
      viewPort = NULL;
    }
  }
  return viewPort;
}

void DeleteViewPort(struct ViewPort *viewPort) {
  FreeVPortCopLists(viewPort);
  DELETE(viewPort->RasInfo);
  DELETE(viewPort);
}

struct View *NewView()
{
  struct View *view;
  
  if ((view = NEW_SZ(struct View)))
    InitView(view);

  return view;
}

void DeleteView(struct View *view) {
  FreeCprList(view->LOFCprList);

  if (view->SHFCprList)
    FreeCprList(view->SHFCprList);

  DELETE(view);
}

static struct Palette {
  UWORD Count;
  UWORD Start;
  ULONG Components[768];
} Palette;

void ViewPortLoadPalette(struct ViewPort *viewPort, UBYTE *components,
                         UWORD start, UWORD count) {
  int i;

  Palette.Count = count;
  Palette.Start = start;

  for (i = 0; i < count * 3; i++)
    Palette.Components[i] = (ULONG)components[i] << 24;

  LoadRGB32(viewPort, (ULONG *)&Palette);
}
