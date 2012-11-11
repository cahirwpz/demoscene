#include <stdarg.h>

#include <exec/types.h>
#include <inline/exec.h>
#include <inline/dos.h> 

#include "startup.h"
#include "print.h"

#define BUFLEN 80

static LONG length = 0;
static BPTR console = 0;

static __regargs void WriteToConsole(char *buffer) {
  if (!console)
    console = Output();

  Write(console, buffer, length);

  length = 0;
}

static void OutputToConsole(char c asm("d0"), char *buffer asm("a3")) {
  buffer[length++] = c;

  if (length == BUFLEN)
    WriteToConsole(buffer);
}

void Print(const char *format, ...) {
  char buffer[BUFLEN];
  va_list args;

  va_start(args, format);
  RawDoFmt(format, args, (void (*)())OutputToConsole, buffer);
  va_end(args);

  WriteToConsole(buffer);
}
