#ifndef __FONT_H__
#define __FONT_H__

#include "gfx.h"

#define CHARMAP_SIZE ('~' - '!' + 1)

typedef struct {
  UWORD y;
  UWORD width;
} FontCharT;

typedef struct {
  BitmapT *data;
  UWORD height;
  UWORD space;
  FontCharT charmap[CHARMAP_SIZE];
} FontT;

typedef struct {
  FontT *font;
  BitmapT *bm;
  Area2D *area;
  UWORD bpl;
} FontDrawCtxT;

__regargs void DrawTextN(FontDrawCtxT *ctx, UBYTE *text, UWORD n);
__regargs Size2D DrawTextSizeN(FontT *font, UBYTE *text, UWORD n);

#define DrawText(font, text) DrawTextN((font), (text), 65535);
#define DrawTextSize(font, text) DrawTextSizeN((font), (text), 65535);

__regargs FontT *LoadFont(char *filename);
__regargs void DeleteFont(FontT *font);

#endif
