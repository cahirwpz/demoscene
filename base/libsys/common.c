#include "rawio.h"
#include "hardware.h"
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

__regargs void MemDump(void *ptr, int n) {
  char *data = ptr;

  while (n > 0) {
    short m = min(n, 16);
    short i = 0;

    DPutChar('$');
    DPutLong((int)data);
    DPutChar(':');

    while (m--) {
      if ((i++ & 3) == 0)
        DPutChar(' ');

      DPutByte(*data++);

      n--;
    }

    DPutChar('\n');
  }
}
