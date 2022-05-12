#ifndef __SYSTEM_MOUSE_H__
#define __SYSTEM_MOUSE_H__

#include <gfx.h>

#define LMB_PRESSED  1
#define RMB_PRESSED  2
#define LMB_RELEASED 4
#define RMB_RELEASED 8

#include <system/syscall.h>

SCARG1NR(MouseInit, Box2D *, win, a0);
SCARG0NR(MouseKill);

#endif /* !__SYSTEM_MOUSE_H__ */
