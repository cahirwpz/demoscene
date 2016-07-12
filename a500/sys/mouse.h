#ifndef __MOUSE_H__
#define __MOUSE_H__

#include "common.h"

#define EV_MOUSE 2

#define LMB_PRESSED  1
#define RMB_PRESSED  2
#define LMB_RELEASED 4
#define RMB_RELEASED 8

typedef struct MouseEvent {
  UBYTE type;
  UBYTE button;
  WORD  x, y;
  BYTE  xrel, yrel;
} MouseEventT;

__regargs void MouseInit(WORD minX, WORD minY, WORD maxX, WORD maxY);
void MouseKill();

#endif
