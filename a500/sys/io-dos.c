#include <stdarg.h>
#include <proto/exec.h>
#include <proto/dos.h> 

#include "io.h"

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

FileT OpenFile(CONST STRPTR path asm("d1")) {
  return Open(path, MODE_OLDFILE);
}

void CloseFile(FileT fh asm("d1")) {
  Close(fh);
}

BOOL FileRead(FileT fh asm("d1"), APTR buf asm("d2"), LONG size asm("d3")) {
  return (Read(fh, buf, size) == size);
}

LONG FileSeek(FileT fh asm("d1"), LONG pos asm("d2"), LONG mode asm("d3")) {
  return Seek(fh, pos, mode);
}

LONG GetFileSize(CONST STRPTR path asm("d1")) {
  LONG size = -1;
  BPTR fh;

  if ((fh = Lock(path, ACCESS_READ))) {
    struct FileInfoBlock fib;

    if (Examine(fh, &fib))
      size = fib.fib_Size;

    UnLock(fh);
  }

  return size;
}

STRPTR __cwdpath; /* symbol is defined in common area and can be overridden */
static BPTR oldcwd = NULL;

void InitIoDos() {
  if (__cwdpath) {
    Log("[Init] Current work dir: \"%s\"\n", __cwdpath);
    oldcwd = CurrentDir(Lock(__cwdpath, ACCESS_READ));
  }
}

void KillIoDos() {
  if (oldcwd) {
    Log("[Quit] Restoring original work dir.\n");
    UnLock(CurrentDir(oldcwd));
  }
}

ADD2INIT(InitIoDos, -10);
ADD2EXIT(KillIoDos, -10);
