#include <stdarg.h>
#include <proto/exec.h>

#include "hardware.h"

#define BAUD 9600
#define CLOCK 3546895

void Log(const char *format, ...) {
  va_list args;

  custom->serper = CLOCK / BAUD - 1;

  va_start(args, format);
  RawDoFmt(format, args, (void (*)())KPutChar, NULL);
  va_end(args);
}
