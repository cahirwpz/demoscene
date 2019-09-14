#include "config.h"
#include "io.h"

#if USE_IO_DOS

#include <proto/dos.h> 

#define BUFLEN 80

typedef struct ConBuf {
  BPTR fh;
  WORD length;
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

void Print(const char *format, ...) {
  va_list args;

  conbuf->fh = Output();
  conbuf->length = 0;

  va_start(args, format);
  RawDoFmt(format, args, (void (*)())DosPutChar, (void *)conbuf);
  va_end(args);

  Write(conbuf->fh, conbuf->data, conbuf->length);
}

#endif
