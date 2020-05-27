#include "rawio.h"
#include "common.h"

void Log(const char *format, ...) {
  va_list args;

  va_start(args, format);
  kvprintf(format, (kvprintf_fn_t *)DPutChar, NULL, args);
  va_end(args);
}

__noreturn void Panic(const char *format, ...) {
  va_list args;

  va_start(args, format);
  kvprintf(format, (kvprintf_fn_t *)DPutChar, NULL, args);
  va_end(args);

  exit(10);
}
