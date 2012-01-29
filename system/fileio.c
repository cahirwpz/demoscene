#include <dos/dos.h>

#include <proto/exec.h>
#include <proto/dos.h>

#include <inline/exec_protos.h>
#include <inline/dos_protos.h>

APTR ReadFileSimple(STRPTR fileName, ULONG memFlags) {
  BPTR fh;
  APTR data = NULL;
  
  if ((fh = Open(fileName, MODE_OLDFILE))) {
    struct FileInfoBlock *infoBlock;

    if ((infoBlock = (struct FileInfoBlock *) AllocDosObject(DOS_FIB, NULL))) {
      if (ExamineFH(fh, infoBlock)) {
        ULONG dataLen = infoBlock->fib_Size;

        if ((data = AllocVec(dataLen, memFlags))) {
          if (dataLen =! Read(fh, data, dataLen)) {
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
