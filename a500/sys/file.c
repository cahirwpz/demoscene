#include <dos/dos.h>
#include <proto/exec.h>
#include <proto/dos.h>

#include "common.h"
#include "file.h"
#include "memory.h"
#include "print.h"

APTR ReadFile(CONST STRPTR path, ULONG memoryFlags) {
  BYTE *data = NULL;
  LONG size = -1;
  BPTR fh;

  if ((fh = Lock(path, ACCESS_READ))) {
    struct FileInfoBlock fib;

    if (Examine(fh, &fib))
      size = fib.fib_Size;

    UnLock(fh);
  }
  
  if ((size > 0) && (fh = Open(path, MODE_OLDFILE))) {
    if ((data = MemAllocAuto(size + 1, memoryFlags))) {
      if (size != Read(fh, data, size)) {
        MemFreeAuto(data);
        data = NULL;
      }
      /* Add extra byte and mark the end of file by zero. */
      data[size] = 0;
    }
    Close(fh);
  } else {
    Print("File '%s' missing.\n", path);
  }

  return data;
}
