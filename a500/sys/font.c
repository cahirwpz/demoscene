#include "font.h"
#include "blitter.h"

__regargs void DrawTextN(FontDrawCtxT *ctx, UBYTE *text, UWORD n) {
  FontT *font = ctx->font;
  FontCharT *charmap = font->charmap;
  Area2D src = { 0, 0, 0, 0 };
  WORD sx, sy, w, h;
  WORD x = 0, y = 0; 
  UWORD c;

  if (ctx->area) {
    sx = ctx->area->x;
    sy = ctx->area->y;
    w = x + ctx->area->w;
    h = y + ctx->area->h;
  } else {
    sx = 0;
    sy = 0;
    w = ctx->bm->width;
    h = ctx->bm->height;
  }

  src.h = min(font->height, h);

  while ((c = *text++) && (n > 0)) {
    if (c == '\n') {
      x = 0;
      y += font->height;
      if (y >= h) break;
      src.h = min(font->height, h - y);
    } else if (c == ' ') {
      x += font->space;
    } else if (x < w) {
      c -= 33;
      if (c < CHARMAP_SIZE) {
        src.y = charmap[c].y;
        src.w = min(charmap[c].width, w - x);

        if (!src.w)
          continue;

        BlitterCopyArea(ctx->bm, ctx->bpl, sx + x, sy + y, font->data, 0, &src);
        x += src.w;
      }
    }
    n--;
  }
}

__regargs Size2D DrawTextSizeN(FontT *font, UBYTE *text, UWORD n) {
  FontCharT *charmap = font->charmap;
  Size2D size = {0, font->height};
  WORD c, x = 0;

  while ((c = *text++) && (n > 0)) {
    if (c == '\n') {
      if (x > size.w)
        size.w = x;
      size.h += font->height;
      x = 0;
    } else if (c == ' ') {
      x += font->space;
    } else {
      c -= 33;
      if (c < CHARMAP_SIZE)
        x += charmap[c].width;
    }
    n--;
  }

  if (x > size.w)
    size.w = x;

  return size;
}
