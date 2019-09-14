#ifndef __SERIAL_H__
#define __SERIAL_H__

#include "common.h"

__regargs void SerialInit(int baud);
void SerialKill(void);
int SerialGet(void);
__regargs void SerialPut(u_char data);
void SerialPrint(const char *format, ...)
  __attribute__ ((format (printf, 1, 2)));

#endif
