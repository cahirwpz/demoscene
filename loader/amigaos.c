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
#include <cpu.h>
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
static BootDataT BootData = {
  .bd_hunk = NULL,
  .bd_vbr = NULL,
  .bd_cpumodel = CPU_68000,
  .bd_stkbot = NULL,
  .bd_stksz = 0,
  .bd_nregions = 2,
  .bd_region = {
    {
      .mr_lower = 0,
      .mr_upper = 0,
      .mr_attr = MEMF_PUBLIC,
    },
    {
      .mr_lower = 0,
      .mr_upper = 0,
      .mr_attr = MEMF_CHIP,
    }
  }
};

/* Some shortcut macros. */
#define ExecVer (SysBase->LibNode.lib_Version)

BootDataT *SaveOS(void) {
  short kickVer, kickRev;
  CpuModelT cpu = CPU_68000;

  Log("[Startup] Save AmigaOS state.\n");

  if (SysBase->AttnFlags & AFF_68060)
    cpu = CPU_68060;
  else if (SysBase->AttnFlags & AFF_68040)
    cpu = CPU_68040;
  else if (SysBase->AttnFlags & AFF_68030)
    cpu = CPU_68030;
  else if (SysBase->AttnFlags & AFF_68020)
    cpu = CPU_68020;
  else if (SysBase->AttnFlags & AFF_68010)
    cpu = CPU_68010;

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

  BootData.bd_vbr = NULL;
  BootData.bd_cpumodel = cpu;

  {
    MemRegionT *mr = &BootData.bd_region[1];
    uintptr_t ptr = (uintptr_t)AllocMem(CHIPMEM, mr->mr_attr);
    mr->mr_lower = ptr;
    mr->mr_upper = ptr + CHIPMEM;
  }

  {
    MemRegionT *mr = &BootData.bd_region[0];
    uintptr_t ptr = (uintptr_t)AllocMem(FASTMEM, MEMF_PUBLIC);
    mr->mr_lower = ptr;
    mr->mr_upper = ptr + FASTMEM;
  }

  return &BootData;
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
