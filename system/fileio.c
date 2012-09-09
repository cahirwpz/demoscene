#include <dos/dos.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <exec/memory.h>

#include "std/memory.h"
#include "system/fileio.h"

bool InitFileIo() {
  BPTR lock = GetProgramDir();

  /* Change current directory to the program's directory, to ensure that
   * relative paths work the same way. */
  return CurrentDir(lock) != -1;
}

static PtrT ReadFileToCustomMemory(const StrT fileName, uint32_t memFlags) {
  BPTR fh;
  PtrT data = NULL;
  
  if ((fh = Open(fileName, MODE_OLDFILE))) {
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

  return data;
}

PtrT ReadFileToChipMem(const StrT fileName) {
  return ReadFileToCustomMemory(fileName, MEMF_CHIP);
}

PtrT ReadFileSimple(const StrT fileName) {
  return ReadFileToCustomMemory(fileName, MEMF_PUBLIC);
}
