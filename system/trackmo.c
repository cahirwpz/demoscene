#include <linkerset.h>
#include <debug.h>
#include <system/boot.h>
#include <system/filesys.h>
#include <system/file.h>
#include <system/floppy.h>
#include <system/memfile.h>

void InitTrackmo(void) {
  FileT *dev = NULL;
  if (BootDev == 0) {
    dev = FloppyOpen();
  } else {
    PANIC();
  }
  InitFileSys(dev);
}

void KillTrackmo(void) {
  KillFileSys();
}

ADD2INIT(InitTrackmo, 0);
ADD2EXIT(KillTrackmo, 0);
