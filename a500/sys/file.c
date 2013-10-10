#include <dos/dos.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <exec/memory.h>

#include "file.h"
#include "print.h"

FileDataT *ReadFile(STRPTR path, ULONG memoryFlags) {
  FileDataT *data = NULL;
  LONG size = -1;
  BPTR fh;

  if ((fh = Lock(path, ACCESS_READ))) {
    struct FileInfoBlock fib;

    if (Examine(fh, &fib))
      size = fib.fib_Size;

    UnLock(fh);
  }
  
  if ((size > 0) && (fh = Open(path, MODE_OLDFILE))) {
    if ((data = AllocMem(size + sizeof(FileDataT), memoryFlags))) {
      data->size = size;
      if (size != Read(fh, data->data, size)) {
        FreeFile(data);
        data = NULL;
      }
    }
    Close(fh);
  }

  return data;
}

void FreeFile(FileDataT *data) {
  FreeMem(data, data->size + sizeof(FileDataT));
}
