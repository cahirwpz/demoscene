#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <graphics/gfx.h>
#include <graphics/view.h>

struct ViewPort *NewViewPort(struct ColorMap *colormap, struct BitMap *bitmap,
                             SHORT width, SHORT height);
void DeleteViewPort(struct ViewPort *viewPort);
void ViewPortLoadPalette(struct ViewPort *viewPort, UBYTE *components,
                         UWORD start, UWORD count);

struct View *NewView();
void DeleteView(struct View *view);

#endif
