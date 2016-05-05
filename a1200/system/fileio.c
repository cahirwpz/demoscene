#include <dos/dos.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <exec/memory.h>
#include <string.h>

#include "std/memory.h"
#include "system/fileio.h"

static PtrT ReadFileToMemory(const char *path, bool text) {
  BPTR fh;
  PtrT data = NULL;
  
  if ((fh = Open(path, MODE_OLDFILE))) {
    struct FileInfoBlock *infoBlock;

    if ((infoBlock = (struct FileInfoBlock *) AllocDosObject(DOS_FIB, NULL))) {
      if (ExamineFH(fh, infoBlock)) {
        size_t dataLen = infoBlock->fib_Size;

        if ((data = MemNew(dataLen + (text ? 1 : 0)))) {
          if (dataLen != Read(fh, data, dataLen)) {
            MemUnref(data);
            data = NULL;
          }
        }
      }
      FreeDosObject(DOS_FIB, infoBlock);
    }
    Close(fh);
  }

  return data;
}

void *ReadFileSimple(const char *fileName) {
  return ReadFileToMemory(fileName, false);
}

char *ReadTextSimple(const char *fileName) {
  return ReadFileToMemory(fileName, true);
}

void WriteFileSimple(const char *path, PtrT data, size_t length) {
  BPTR fh;

  if ((fh = Open(path, MODE_NEWFILE))) {
    Write(fh, data, length);
    Close(fh);
  }
}
