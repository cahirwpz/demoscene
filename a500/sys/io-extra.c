#include "memory.h"
#include "io.h"

__regargs APTR LoadFile(CONST STRPTR path, ULONG memoryFlags) {
  BYTE *data = NULL;
  LONG size = GetFileSize(path);
  FileT *file;
  
  if ((size > 0) && (file = OpenFile(path, 0))) {
    if ((data = MemAlloc(size + 1, memoryFlags))) {
      if (!FileRead(file, data, size)) {
        MemFree(data);
        data = NULL;
      } else {
        /* Add extra byte and mark the end of file by zero. */
        data[size] = 0;
      }
    }
    CloseFile(file);
  } else {
    Print("File '%s' missing.\n", path);
  }

  return data;
}
