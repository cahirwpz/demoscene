#include <dos/dos.h>
#include <proto/exec.h>
#include <proto/dos.h>

#include "system/fileio.h"

PtrT ReadFileToCustomMemory(const StrT fileName, uint32_t memFlags) {
  BPTR fh;
  PtrT data = NULL;
  
  if ((fh = Open(fileName, MODE_OLDFILE))) {
    struct FileInfoBlock *infoBlock;

    if ((infoBlock = (struct FileInfoBlock *) AllocDosObject(DOS_FIB, NULL))) {
      if (ExamineFH(fh, infoBlock)) {
        size_t dataLen = infoBlock->fib_Size;

        if ((data = AllocVec(dataLen, memFlags))) {
          if (dataLen != Read(fh, data, dataLen)) {
            FreeVec(data);
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

PtrT ReadFileSimple(const StrT fileName) {
  return ReadFileToCustomMemory(fileName, MEMF_PUBLIC);
}
