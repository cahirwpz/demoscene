#include <stdarg.h>
#include <proto/exec.h>

#include "config.h"
#include "rawio.h"
#include "hardware.h"

BOOL execOnly = FALSE;

#if USE_IO_DOS
#include <proto/dos.h> 
#define BUFLEN 80

typedef struct ConBuf {
  BPTR fh;
  WORD length;
  BOOL active;
  char data[BUFLEN];
} ConBufT;

static ConBufT conbuf[1];

static void DosPutChar(char c asm("d0"), ConBufT *cb asm("a3")) {
  DPutChar(c);

  if (cb->active) {
    cb->data[cb->length++] = c;

    if (cb->length == BUFLEN) {
      Write(cb->fh, cb->data, cb->length);
      cb->length = 0;
    }
  }
}
#endif

void Log(const char *format, ...) {
  va_list args;

#if USE_IO_DOS
  conbuf->active = DOSBase && !execOnly;

  if (conbuf->active) {
    conbuf->fh = Output();
    conbuf->length = 0;
  }

  va_start(args, format);
  RawDoFmt(format, args, (void (*)())DosPutChar, (void *)conbuf);
  va_end(args);

  if (conbuf->active)
    Write(conbuf->fh, conbuf->data, conbuf->length);
#else
  va_start(args, format);
  RawDoFmt(format, args, (void (*)())DPutChar, NULL);
  va_end(args);
#endif
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
