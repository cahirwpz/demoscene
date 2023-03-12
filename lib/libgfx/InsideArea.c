#include <gfx.h>

bool InsideArea(short x, short y, const Area2D *area) {
  short x1 = area->x;
  short y1 = area->y;
  short x2 = area->x + area->w - 1;
  short y2 = area->y + area->h - 1;

  return (x1 <= x && x <= x2 && y1 <= y && y <= y2);
}
