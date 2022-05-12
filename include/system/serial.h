#ifndef __SYSTEM_SERIAL_H__
#define __SYSTEM_SERIAL_H__

#include <types.h>

struct File;

#include <system/syscall.h>

SCARG2(SerialOpen, struct File *, u_int, baud, d0, u_int, flags, d1);

#endif
