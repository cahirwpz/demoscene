#include <stdarg.h>
#include <proto/exec.h>

#include "rawio.h"
#include "hardware.h"

void Log(const char *format, ...) {
  va_list args;

  va_start(args, format);
  RawDoFmt(format, args, (void (*)())DPutChar, NULL);
  va_end(args);
}

typedef struct {
  char *str, *end;
} FmtBufT;

static void FmtPutChar(char c asm("d0"), FmtBufT *buf asm("a3")) {
  if (buf->str < buf->end)
    *buf->str++ = c;
}

void FmtStr(char *str, ULONG size, const char *format, ...) {
  va_list args;
  FmtBufT buf[] = {{ str, str + size - 1 }};

  va_start(args, format);
  RawDoFmt(format, args, (void (*)())FmtPutChar, buf);
  va_end(args);

  *buf->str = '\0';
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
