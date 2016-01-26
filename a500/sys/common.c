#include <stdarg.h>
#include <proto/exec.h>

#include "rawio.h"
#include "hardware.h"

#define BAUD 115200
#define CLOCK 3546895

void Log(const char *format, ...) {
  va_list args;

  custom->serper = CLOCK / BAUD - 1;

  va_start(args, format);
  RawDoFmt(format, args, (void (*)())DPutChar, NULL);
  va_end(args);
}

__regargs void MemDump(APTR ptr, LONG n) {
  char *data = ptr;

  while (n > 0) {
    WORD m = min(n, 16);
    WORD i = 0;

    DPutChar('$');
    DPutLong((LONG)data);
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
