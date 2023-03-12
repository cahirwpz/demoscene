#include <linkerset.h>
#include <system/boot.h>
#include <system/filesys.h>
#include <system/file.h>
#include <system/floppy.h>
#include <system/memfile.h>

void InitTrackmo(void) {
  FileT *dev;
  if (BootDev)
    dev = MemOpen((const void *)0xf80000, 0x80000);
  else
    dev = FloppyOpen();
  InitFileSys(dev);
}

void KillTrackmo(void) {
  KillFileSys();
}

ADD2INIT(InitTrackmo, 0);
ADD2EXIT(KillTrackmo, 0);
