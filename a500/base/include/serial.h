#ifndef __SERIAL_H__
#define __SERIAL_H__

#include "common.h"

__regargs void SerialInit(LONG baud);
void SerialKill();
LONG SerialGet();
__regargs void SerialPut(UBYTE data);
void SerialPrint(const char *format, ...)
  __attribute__ ((format (printf, 1, 2)));

#endif
