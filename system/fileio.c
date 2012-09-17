#include <dos/dos.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <exec/memory.h>
#include <string.h>

#include "std/memory.h"
#include "system/fileio.h"

StrT AbsPath(const StrT fileName) {
  StrT path = MemNew(sizeof("PROGDIR:") + strlen(fileName) + 1);
  strcpy(path, "PROGDIR:");
  strcat(path, fileName);
  return path;
}

static PtrT ReadFileToCustomMemory(const StrT fileName, uint32_t memFlags) {
  BPTR fh;
  PtrT data = NULL;
  PtrT path = AbsPath(fileName);
  
  if ((fh = Open(path, MODE_OLDFILE))) {
    struct FileInfoBlock *infoBlock;

    if ((infoBlock = (struct FileInfoBlock *) AllocDosObject(DOS_FIB, NULL))) {
      if (ExamineFH(fh, infoBlock)) {
        size_t dataLen = infoBlock->fib_Size;

        if ((data = MemNewCustom(dataLen, memFlags, NULL))) {
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

  MemUnref(path);

  return data;
}

PtrT ReadFileToChipMem(const StrT fileName) {
  return ReadFileToCustomMemory(fileName, MEMF_CHIP);
}

PtrT ReadFileSimple(const StrT fileName) {
  return ReadFileToCustomMemory(fileName, MEMF_PUBLIC);
}

void WriteFileSimple(const StrT fileName, PtrT data, size_t length) {
  BPTR fh;
  PtrT path = AbsPath(fileName);

  if ((fh = Open(path, MODE_NEWFILE))) {
    Write(fh, data, length);
    Close(fh);
  }
}
