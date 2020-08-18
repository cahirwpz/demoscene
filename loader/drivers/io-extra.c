#include "memory.h"
#include "filesys.h"

void *LoadFile(const char *path, u_int memoryFlags) {
  char *data = NULL;
  int size = GetFileSize(path);

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
