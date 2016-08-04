#ifndef __MOUSE_H__
#define __MOUSE_H__

#include "common.h"

#define LMB_PRESSED  1
#define RMB_PRESSED  2
#define LMB_RELEASED 4
#define RMB_RELEASED 8

__regargs void MouseInit(WORD minX, WORD minY, WORD maxX, WORD maxY);
void MouseKill();

#endif
