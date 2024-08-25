#include <linkerset.h>
#include <system/boot.h>
#include <system/filesys.h>
#include <system/file.h>
#include <system/floppy.h>
#include <system/memfile.h>

#define ROMADDR 0xf80000
#define ROMSIZE 0x07fff0
#define ROMEXTADDR 0xe00000
#define ROMEXTSIZE 0x080000

static const MemBlockT rom[] = {
  {(const void *)ROMADDR, ROMSIZE},
  {(const void *)ROMEXTADDR, ROMEXTSIZE},
  {NULL, 0}
};

void InitTrackmo(void) {
  FileT *dev;
  if (BootDev)
    dev = MemOpen(rom);
  else
    dev = FloppyOpen();
  InitFileSys(dev);
}

void KillTrackmo(void) {
  KillFileSys();
}

ADD2INIT(InitTrackmo, 0);
ADD2EXIT(KillTrackmo, 0);
