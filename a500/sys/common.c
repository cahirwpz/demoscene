#include <stdarg.h>
#include <proto/exec.h>

#include "hardware.h"

#define BAUD 9600
#define CLOCK 3546895

#define TBE (1 << 13)
#define TSRE (1 << 12)

static void OutputToSerial(char c asm("d0"), APTR *userdata asm("a3")) {
  while (!(custom->serdatr & TBE));
  custom->serdat = c | 0x100;

  if (c == '\n') {
    while (!(custom->serdatr & TBE));
    custom->serdat = '\r' | 0x100;
  }
}

void Log(const char *format, ...) {
  va_list args;

  custom->serper = CLOCK / BAUD - 1;

  va_start(args, format);
  RawDoFmt(format, args, (void (*)())OutputToSerial, NULL);
  va_end(args);
}
