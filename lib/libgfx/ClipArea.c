#include <gfx.h>

bool ClipArea(const Box2D *space, Point2D *pos, Area2D *area) {
  short minX = space->minX;
  short minY = space->minY;
  short maxX = space->maxX;
  short maxY = space->maxY;
  short posX = pos->x;
  short posY = pos->y;

  if ((posX + area->w <= minX) || (posX > maxX))
    return false;
  if ((posY + area->h <= minY) || (posY > maxY))
    return false;

  if (posX < minX) {
    area->x += minX - posX;
    area->w -= minX - posX;
    pos->x = posX = minX;
  }

  if (posY < minY) {
    area->y += minY - posY;
    area->h -= minY - posY;
    pos->y = posY = minY;
  }

  if (posX + area->w > maxX)
    area->w = maxX - posX + 1;

  if (posY + area->h > maxY)
    area->h = maxY - posY + 1;

  return true;
}
