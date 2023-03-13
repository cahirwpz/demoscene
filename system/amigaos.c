#include <exec/execbase.h>
#include <graphics/gfxbase.h>

#include <hardware/adkbits.h>
#include <hardware/custom.h>
#include <hardware/dmabits.h>
#include <hardware/intbits.h>
#include <proto/alib.h>
#include <proto/exec.h>
#undef Debug
#include <proto/graphics.h>

#include <system/boot.h>
#include <system/cpu.h>
#include <system/exception.h>

#include <debug.h>
#include <string.h>
#include <strings.h>

/* Use _custom definition provided by the linker. */
extern struct Custom volatile _custom;
#define custom (&_custom)

/* We need graphics.library base in order to call some functions. */
static struct GfxBase *__CONSTLIBBASEDECL__ GfxBase;

static void WaitVBlank(void) {
  const uint32_t line = 303;
  volatile uint32_t *vposr = (u_int *)&custom->vposr;
  while ((*vposr & 0x1ff00) < ((line << 8) & 0x1ff00))
    continue;
}

/* AmigaOS state that we want to preserve. */
static struct View *oldView;
static u_short oldDmacon, oldIntena, oldAdkcon;
static u_int oldCacheBits;
static ExcVecT oldExcVec;
static void *oldSysStack;

/* Memory for framework allocator. */
static __aligned(4) __bss_chip char ChipMem[CHIPMEM_KB * 1024];
static __aligned(4) char FastMem[FASTMEM_KB * 1024];

/* Normally BootDataT is provided by the boot loader. Since we were started
 * from AmigaOS we have to fill this structure and pass it to Loader. */
static BootDataT BootData = {
  .bd_hunk = NULL,
  .bd_vbr = NULL,
  .bd_bootdev = 2,
  .bd_cpumodel = CPU_68000,
  .bd_stkbot = NULL,
  .bd_stksz = 0,
  .bd_nregions = 2,
  .bd_region = {
    {
      .mr_lower = (uintptr_t)FastMem,
      .mr_upper = (uintptr_t)FastMem + sizeof(FastMem), 
      .mr_attr = MEMF_PUBLIC,
    },
    {
      .mr_lower = (uintptr_t)ChipMem,
      .mr_upper = (uintptr_t)ChipMem + sizeof(ChipMem),
      .mr_attr = MEMF_CHIP,
    }
  }
};

/* Some shortcut macros. */
#define ExecVer (SysBase->LibNode.lib_Version)

/* Linker exported symbols (see amiga.ld linker script). */
extern char _bss[];
extern char _bss_size[];
extern char _bss_chip[];
extern char _bss_chip_size[];

BootDataT *SaveOS(void) {
  BootDataT *bd = &BootData;

  Log("[Startup] Save AmigaOS state.\n");

  /* KS 1.3 and earlier are brain-dead since they don't clear BSS sections :( */
  if (ExecVer <= 34) {
    bzero(_bss, (size_t)_bss_size);
    bzero(_bss_chip, (size_t)_bss_chip_size);
  }

  /* Workaround for const-ness of GfxBase declaration. */
  *(struct GfxBase **)&GfxBase =
    (struct GfxBase *)OpenLibrary("graphics.library", 33);

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
  WaitVBlank();

  /* Clear all interrupt requests. Really. */
  custom->intreq = (UWORD)~INTF_SETCLR;
  custom->intreq = (UWORD)~INTF_SETCLR;

  {
    struct Task *self = FindTask(NULL);
    bd->bd_stkbot = self->tc_SPLower;
    bd->bd_stksz = self->tc_SPUpper - self->tc_SPLower;
  }

  /* Enter supervisor mode and save exception vector
   * since the framework takes full control over it. */
  oldSysStack = SuperState();

  /* Detect CPU model and fetch VBR on 68010 and later. */
  {
    CpuModelT cpu = CPU_68000;

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

    if (cpu > CPU_68000) {
      void *ptr;
      asm("movec vbr, %0" : "=d" (ptr));
      bd->bd_vbr = ptr;
    }

    bd->bd_cpumodel = cpu;
  }

  memcpy(oldExcVec, bd->bd_vbr, sizeof(oldExcVec));

  return &BootData;
}

void RestoreOS(void) {
  BootDataT *bd = &BootData;

  Log("[Startup] Restore AmigaOS state.\n");

  /* firstly... disable dma and interrupts that were used in Main */
  custom->dmacon = (UWORD)~DMAF_SETCLR;
  custom->intena = (UWORD)~INTF_SETCLR;
  WaitVBlank();

  /* Clear all interrupt requests. Really. */
  custom->intreq = (UWORD)~INTF_SETCLR;
  custom->intreq = (UWORD)~INTF_SETCLR;

  /* Restore exception vector and leave supervisor mode. */
  memcpy(bd->bd_vbr, oldExcVec, sizeof(oldExcVec));
  UserState(oldSysStack);

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

  CloseLibrary((struct Library *)GfxBase);
}
