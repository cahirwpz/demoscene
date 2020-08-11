#ifndef __SERIAL_H__
#define __SERIAL_H__

#include "common.h"

void SerialInit(int baud);
void SerialKill(void);
int SerialGet(void);
void SerialPut(u_char data);
void SerialPrint(const char *format, ...)
  __attribute__ ((format (printf, 1, 2)));

#endif
