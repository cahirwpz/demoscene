#include "debug.h"

#ifndef UAE
#include <stdarg.h>
#include <stdio.h>

extern void DPutChar(void *ptr, char data);

void Log(const char *format, ...) {
  va_list args;

  va_start(args, format);
  kvprintf(DPutChar, (void *)ciab, format, args);
  va_end(args);
}

__noreturn void Panic(const char *format, ...) {
  va_list args;

  va_start(args, format);
  kvprintf(DPutChar, (void *)ciab, format, args);
  va_end(args);

  PANIC();
  for (;;);
}
#endif
