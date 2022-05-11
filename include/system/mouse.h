#ifndef __SYSTEM_MOUSE_H__
#define __SYSTEM_MOUSE_H__

#define LMB_PRESSED  1
#define RMB_PRESSED  2
#define LMB_RELEASED 4
#define RMB_RELEASED 8

#include <system/syscall.h>

#ifdef _SYSTEM
void MouseInit(short minX, short minY, short maxX, short maxY);
#endif

SCARG0NR(MouseKill);

#endif /* !__SYSTEM_MOUSE_H__ */
