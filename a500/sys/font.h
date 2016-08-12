#ifndef __FONT_H__
#define __FONT_H__

#include "gfx.h"

#define CHARMAP_SIZE ('~' - '!' + 1)

typedef struct {
  BitmapT *data;
  UWORD height;
  UWORD space;
  struct {
    UWORD y;
    UWORD width;
  } charmap[CHARMAP_SIZE];
} FontT;

__regargs void DrawTextSetup(BitmapT *dst, UWORD dstbpl, FontT *font);
__regargs void DrawText(WORD x, WORD y, UBYTE *text);

__regargs FontT *LoadFont(char *filename);
__regargs void DeleteFont(FontT *font);

#endif
