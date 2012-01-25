#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <graphics/gfx.h>
#include <graphics/view.h>

struct DBufRaster {
  struct ViewPort *ViewPort;
  struct DBufInfo *DBufInfo;
};

struct DBufRaster *NewDBufRaster(SHORT width, SHORT height, SHORT depth);
void DeleteDBufRaster(struct DBufRaster *raster);

void ViewPortLoadPalette(struct ViewPort *viewPort, UBYTE *components,
                         UWORD start, UWORD count);

struct View *NewView();
void DeleteView(struct View *view);

#endif
