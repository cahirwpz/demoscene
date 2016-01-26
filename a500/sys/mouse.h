#ifndef __MOUSE_H__
#define __MOUSE_H__

#include "common.h"

typedef enum {
  LMB_PRESSED  = 1,
  RMB_PRESSED  = 2,
  LMB_RELEASED = 4,
  RMB_RELEASED = 8
} __attribute__((packed)) MouseButtonT;

typedef struct {
  WORD x, y;
  BYTE xrel, yrel;
  MouseButtonT button;
} MouseEventT;

__regargs void MouseInit(WORD minX, WORD minY, WORD maxX, WORD maxY);
void MouseKill();
__regargs BOOL GetMouseEvent(MouseEventT *event);

#endif
