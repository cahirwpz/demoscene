#ifndef __SYSTEM_SERIAL_H__
#define __SYSTEM_SERIAL_H__

#include <types.h>

struct File;

struct File *OpenSerial(u_int baud, u_int flags);

#endif /* !__SYSTEM_SERIAL_H__ */
