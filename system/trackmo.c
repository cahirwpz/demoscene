#include <linkerset.h>
#include <debug.h>
#include <system/boot.h>
#include <system/filesys.h>
#include <system/file.h>
#include <system/floppy.h>
#include <system/memfile.h>

void InitTrackmo(void) {
  FileT *dev = NULL;
  if (BootDev != 0xff) {
    dev = FloppyOpen(BootDev);
  } else {
    Panic("[Trackmo] Not configured to run from: %d!", BootDev);
  }
  InitFileSys(dev);
}

void CheckTrackmo(void) {
  CheckFileSys();
}

void KillTrackmo(void) {
  KillFileSys();
}

ADD2INIT(InitTrackmo, 0);
ADD2EXIT(KillTrackmo, 0);
