#ifndef __MOUSE_H__
#define __MOUSE_H__

#include "common.h"

#define LMB_PRESSED  1
#define RMB_PRESSED  2
#define LMB_RELEASED 4
#define RMB_RELEASED 8

void MouseInit(short minX, short minY, short maxX, short maxY);
void MouseKill(void);

#endif
