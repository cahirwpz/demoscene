#include "font.h"
#include "bltop.h"

static struct {
  BitmapT *dst;
  FontT *font;
  UWORD dstbpl;
} state[1];

__regargs void DrawTextSetup(BitmapT *dst, UWORD dstbpl, FontT *font) {
  state->dst = dst;
  state->dstbpl = dstbpl;
  state->font = font;
}

__regargs void DrawText(WORD x, WORD y, UBYTE *text) {
  FontT *font = state->font;
  Area2D area = { 0, 0, 0, font->height };
  WORD px = x;
  UWORD c;

  while ((c = *text++)) {
    if (c == '\n') {
      x = px, y += font->height;
    } else if (c == ' ') {
      x += font->space;
    } else {
      c -= 33;
      if (c < CHARMAP_SIZE) {
        area.y = font->charmap[c].y;
        area.w = font->charmap[c].width;

        if (!area.w)
          continue;

        BlitterCopyArea(state->dst, state->dstbpl, x, y, font->data, 0, &area);
        x += area.w;
      }
    }
  }
}

