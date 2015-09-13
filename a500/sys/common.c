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

void MemDump(APTR ptr, LONG n) {
  char *data = ptr;
  char line[16 * 3];

  while (n > 0) {
    char *orig = data;
    char *s = line;
    WORD m = min(n, 16);
    WORD i;

    for (i = 0; i < m; i++, n--) {
      char byte = *data++;
      char hi = (byte >> 4) & 15;
      char lo = byte & 15;

      *s++ = (hi >= 10) ? (hi + 'A' - 10) : (hi + '0');
      *s++ = (lo >= 10) ? (lo + 'A' - 10) : (lo + '0');
      *s++ = ' ';
    }

    s[-1] = '\0';
    data += m;

    Log("%08lx : %s\n", (LONG)orig, line);
  }
}
