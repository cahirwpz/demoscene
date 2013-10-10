#include <dos/dos.h>
#include <proto/exec.h>
#include <proto/dos.h>

#include "file.h"
#include "memory.h"

APTR ReadFile(STRPTR path, ULONG memoryFlags) {
  APTR data = NULL;
  LONG size = -1;
  BPTR fh;

  if ((fh = Lock(path, ACCESS_READ))) {
    struct FileInfoBlock fib;

    if (Examine(fh, &fib))
      size = fib.fib_Size;

    UnLock(fh);
  }
  
  if ((size > 0) && (fh = Open(path, MODE_OLDFILE))) {
    if ((data = AllocAutoMem(size, memoryFlags))) {
      if (size != Read(fh, data, size)) {
        FreeAutoMem(data);
        data = NULL;
      }
    }
    Close(fh);
  }

  return data;
}
