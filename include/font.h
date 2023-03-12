#ifndef __FONT_H__
#define __FONT_H__

#include "gfx.h"

#define CHARMAP_SIZE ('~' - '!' + 1)

typedef struct {
  u_short y;
  u_short width;
} FontCharT;

typedef struct {
  BitmapT *data;
  u_short height;
  u_short space;
  FontCharT charmap[CHARMAP_SIZE];
} FontT;

typedef struct {
  FontT *font;
  BitmapT *bm;
  Area2D *area;
  u_short bpl;
} FontDrawCtxT;

void DrawTextN(FontDrawCtxT *ctx, const char *text, u_short n);
Size2D DrawTextSizeN(FontT *font, const char *text, u_short n);

#define DrawText(font, text) DrawTextN((font), (text), 65535);
#define DrawTextSize(font, text) DrawTextSizeN((font), (text), 65535);

#endif
