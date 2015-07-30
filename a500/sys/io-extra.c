#include "memory.h"
#include "io.h"

__regargs APTR LoadFile(CONST STRPTR path, ULONG memoryFlags) {
  BYTE *data = NULL;
  LONG size = GetFileSize(path);
  FileT fh;
  
  if ((size > 0) && (fh = OpenFile(path))) {
    if ((data = MemAllocAuto(size + 1, memoryFlags))) {
      if (FileRead(fh, data, size)) {
        MemFreeAuto(data);
        data = NULL;
      }
      /* Add extra byte and mark the end of file by zero. */
      data[size] = 0;
    }
    CloseFile(fh);
  } else {
    Print("File '%s' missing.\n", path);
  }

  return data;
}
