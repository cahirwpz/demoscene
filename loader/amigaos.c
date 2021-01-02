#include <exec/execbase.h>
#include <graphics/gfxbase.h>

#include <hardware/adkbits.h>
#include <hardware/custom.h>
#include <hardware/dmabits.h>
#include <hardware/intbits.h>
#include <proto/alib.h>
#include <proto/exec.h>
#include <proto/graphics.h>

#include <boot.h>
#include <debug.h>

#define CHIPMEM (512 * 1024)
#define FASTMEM (512 * 1024)

/* Use _custom definition provided by the linker. */
extern struct Custom volatile _custom;
#define custom (&_custom)

/* We need graphics.library base in order to call some functions. */
struct GfxBase *__CONSTLIBBASEDECL__ GfxBase;

/* AmigaOS state that we want to preserve. */
static struct View *oldView;
static u_short oldDmacon, oldIntena, oldAdkcon;
static u_int oldCacheBits;

/* Normally BootDataT is provided by the boot loader. Since we were launched
 * from AmigaOS we have to fill this structure and pass it to Loader. */
#define BootDataSize(mr) (sizeof(BootDataT) + sizeof(MemRegionT) * (mr))
static short _bootdata[BootDataSize(2) / sizeof(short)];

/* Some shortcut macros. */
#define ExecVer (SysBase->LibNode.lib_Version)

BootDataT *SaveOS(void) {
  short kickVer, kickRev;
  short cpu = 0;

  Log("[Startup] Save AmigaOS state.\n");

  if (SysBase->AttnFlags & AFF_68060)
    cpu = 6;
  else if (SysBase->AttnFlags & AFF_68040)
    cpu = 4;
  else if (SysBase->AttnFlags & AFF_68030)
    cpu = 3;
  else if (SysBase->AttnFlags & AFF_68020)
    cpu = 2;
  else if (SysBase->AttnFlags & AFF_68010)
    cpu = 1;

  /* Based on WhichAmiga method. */
  {
    APTR kickEnd = (APTR)0x1000000;
    ULONG kickSize = *(ULONG *)(kickEnd - 0x14);
    UWORD *kick = kickEnd - kickSize;

    kickVer = kick[6];
    kickRev = kick[7];
  }

  Log("[Main] ROM: %ld.%ld, CPU: 680%ld0, CHIP: %ldkB, FAST: %ldkB\n",
      (LONG)kickVer, (LONG)kickRev, (LONG)cpu,
      (LONG)(AvailMem(MEMF_CHIP | MEMF_LARGEST) / 1024),
      (LONG)(AvailMem(MEMF_FAST | MEMF_LARGEST) / 1024));

  /* Allocate blitter. */
  WaitBlit();
  OwnBlitter();

  /* Disable multitasking. */
  Forbid();

  /* Disable CPU caches. */
  if (ExecVer >= 36)
    oldCacheBits = CacheControl(0, -1);

  /* Intercept the view of AmigaOS. */
  oldView = GfxBase->ActiView;
  LoadView(NULL);
  WaitTOF();
  WaitTOF();

  /* DMA & interrupts take-over. */
  oldAdkcon = custom->adkconr;
  oldDmacon = custom->dmaconr;
  oldIntena = custom->intenar;

  /* Prohibit dma & interrupts. */
  custom->adkcon = (UWORD)~ADKF_SETCLR;
  custom->dmacon = (UWORD)~DMAF_SETCLR;
  custom->intena = (UWORD)~INTF_SETCLR;
  WaitTOF();

  /* Clear all interrupt requests. Really. */
  custom->intreq = (UWORD)~INTF_SETCLR;
  custom->intreq = (UWORD)~INTF_SETCLR;

  /* Enable master switches...
   * .. and SOFTINT which is presumably used by Exec's scheduler. */
  custom->dmacon = DMAF_SETCLR | DMAF_MASTER;
  custom->intena = INTF_SETCLR | INTF_INTEN | INTF_SOFTINT;

  return (BootDataT *)&_bootdata;
}

void RestoreOS(void) {
  Log("[Startup] Restore AmigaOS state.\n");

  /* Suspend multitasking. */
  Forbid();

  /* firstly... disable dma and interrupts that were used in Main */
  custom->dmacon = (UWORD)~DMAF_SETCLR;
  custom->intena = (UWORD)~INTF_SETCLR;
  WaitTOF();

  /* Clear all interrupt requests. Really. */
  custom->intreq = (UWORD)~INTF_SETCLR;
  custom->intreq = (UWORD)~INTF_SETCLR;

  /* Restore AmigaOS state of dma & interrupts. */
  custom->dmacon = oldDmacon | DMAF_SETCLR;
  custom->intena = oldIntena | INTF_SETCLR;
  custom->adkcon = oldAdkcon | ADKF_SETCLR;

  /* Restore old copper list... */
  custom->cop1lc = (ULONG)GfxBase->copinit;
  WaitTOF();

  /* ... and original view. */
  LoadView(oldView);
  WaitTOF();
  WaitTOF();

  /* Enable CPU caches. */
  if (ExecVer >= 36)
    CacheControl(oldCacheBits, -1);

  /* Restore multitasking. */
  Permit();

  /* Deallocate blitter. */
  DisownBlitter();
}
