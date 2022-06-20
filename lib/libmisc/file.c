#include <stdarg.h>
#include <stdio.h>
#include <system/file.h>
#include <system/errno.h>

int FileGetChar(FileT *f) {
  char c;
  if (FileRead(f, &c, 1) < 1)
    return -1;
  return c;
}

void FilePutChar(FileT *f, char c) {
  FileWrite(f, &c, 1);
}

void FilePrintf(FileT *f, const char *fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  kvprintf((kvprintf_fn_t *)FilePutChar, f, fmt, ap);
  va_end(ap);
}
