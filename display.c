#include <proto/graphics.h>
#include <inline/graphics_protos.h>

#include "common.h"
#include "display.h"

struct ViewPort *NewViewPort(struct BitMap *bitmap,
                             SHORT width, SHORT height, SHORT depth) {
  struct ViewPort *viewPort;
  struct ColorMap *colorMap;

  if ((viewPort = NEW_SZ(struct ViewPort))) {
    InitVPort(viewPort);

    viewPort->DWidth = width;
    viewPort->DHeight = height;
    viewPort->RasInfo = NEW_SZ(struct RasInfo);
    viewPort->ColorMap = GetColorMap(1 << depth);

    if (viewPort->RasInfo && viewPort->ColorMap) {
      viewPort->RasInfo->BitMap = bitmap;
    } else {
      DeleteViewPort(viewPort);
      viewPort = NULL;
    }
  }

  return viewPort;
}

void DeleteViewPort(struct ViewPort *viewPort) {
  FreeVPortCopLists(viewPort);
  FreeColorMap(viewPort->ColorMap);
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
