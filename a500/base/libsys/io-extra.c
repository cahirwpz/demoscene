#include "memory.h"
#include "io.h"

__regargs APTR LoadFile(CONST STRPTR path, ULONG memoryFlags) {
  BYTE *data = NULL;
  LONG size = GetFileSize(path);

  if (size > 0 && (data = MemAlloc(size + 1, memoryFlags))) {
    FileT *file = OpenFile(path, 0);

    if (!FileRead(file, data, size)) {
      MemFree(data);
      data = NULL;
    } else {
      /* Add extra byte and mark the end of file by zero. */
      data[size] = 0;
    }

    CloseFile(file);
  }

  return data;
}
