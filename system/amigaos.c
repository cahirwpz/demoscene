#define __cplusplus
#include <exec/types.h>
#undef __cplusplus
#include <exec/execbase.h>
#define __NOLIBBASE__
#include <graphics/gfxbase.h>

#include <hardware/adkbits.h>
#include <hardware/custom.h>
#include <hardware/dmabits.h>
#include <hardware/intbits.h>
#include <resources/cia.h>
#include <proto/alib.h>
#define LP2NB LP2UB
#include <proto/cia.h>
#include <proto/exec.h>
#undef Debug
#include <proto/graphics.h>

#include <config.h>

#include <system/boot.h>
#include <system/cpu.h>
#include <system/exception.h>
#include <system/cia.h>

#include <debug.h>
#include <string.h>
#include <strings.h>

/* Use _custom definition provided by the linker. */
extern struct Custom volatile _custom;
#define custom (&_custom)

/* We need exec.library & graphics.library base to call some functions. */
extern struct ExecBase *__CONSTLIBBASEDECL__ SysBase;
static struct GfxBase *__CONSTLIBBASEDECL__ GfxBase;

static void WaitVBlank(void) {
  const uint32_t line = 303;
  volatile uint32_t *vposr = (u_int *)&custom->vposr;
  while ((*vposr & 0x1ff00) < ((line << 8) & 0x1ff00))
    continue;
}

typedef struct TimerState {
  u_char cr; /* control register */
  u_char tlo, thi; /* timer values */
  u_char llo, lhi; /* latch values */
} TimerStateT;

typedef volatile u_char *CiaRegT;

static inline void nop(void) {
  asm volatile("nop;" ::: "memory");
}

static inline void bclr(CiaRegT reg, char bit) {
  asm volatile("bclr %1,%0" :: "m" (*reg), "dI" (bit) : "memory");
  nop();
}

static inline void bset(CiaRegT reg, char bit) {
  asm volatile("bset %1,%0" :: "m" (*reg), "dI" (bit) : "memory");
  nop();
}

/* Based on jst_cus.asm from https://github.com/jotd666/jst */
static void GetTimerState(CiaRegT cr asm("a0"), CiaRegT tlo asm("a1"),
                          CiaRegT thi asm("a2"), TimerStateT *ts asm("a3"))
{
  ts->cr = *cr;

  bclr(cr, CIACRB_START);

  ts->tlo = *tlo;
  ts->thi = *thi;

  bclr(cr, CIACRB_RUNMODE);
  bclr(cr, CIACRB_PBON);
  bset(cr, CIACRB_LOAD);

  ts->llo = *tlo;
  ts->lhi = *thi;
}

static void SetTimerState(CiaRegT cr asm("a0"), CiaRegT tlo asm("a1"),
                          CiaRegT thi asm("a2"), TimerStateT *ts asm("a3"))
{
  *cr = 0;
  nop();

  *tlo = ts->tlo;
  *thi = ts->thi;
  nop();

  bset(cr, CIACRB_LOAD);
  *tlo = ts->llo;
  *thi = ts->thi;
  nop();

  *cr = ts->cr;
}

/* AmigaOS state that we want to preserve. */
static __code struct {
  struct View *view;
  u_short dmacon, intena, intreq, adkcon;
  u_int cacheBits;
  ExcVecT excVec;
  void *sysStack;
  TimerStateT timer[4];
  struct Library *resCiaA, *resCiaB;
  u_char icrCiaA, icrCiaB;
} old;

/* Memory for framework allocator. */
static __aligned(4) __bss_chip char ChipMem[CHIPMEM * 1024];
static __aligned(4) char FastMem[FASTMEM * 1024];

/* Normally BootDataT is provided by the boot loader. Since we were started
 * from AmigaOS we have to fill this structure and pass it to Loader. */
static BootDataT BootData = {
  .bd_hunk = NULL,
  .bd_vbr = NULL,
  .bd_topaz = NULL,
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

  {
    struct TextAttr textattr = { (char *)"topaz.font", 8, FS_NORMAL, FPF_ROMFONT };
    struct TextFont *topaz8 = OpenFont(&textattr);
    bd->bd_topaz = topaz8->tf_CharData;
    CloseFont(topaz8);
  }

  old.resCiaA = OpenResource(CIAANAME);
  old.resCiaB = OpenResource(CIABNAME);

  /* Allocate blitter. */
  WaitBlit();
  OwnBlitter();

  /* Disable multitasking. */
  Forbid();

  /* Disable CPU caches. */
  if (ExecVer >= 36)
    old.cacheBits = CacheControl(0, -1);

  /* Intercept the view of AmigaOS. */
  old.view = GfxBase->ActiView;
  LoadView(NULL);
  WaitTOF();
  WaitTOF();

  /* DMA & interrupts take-over. */
  old.adkcon = custom->adkconr;
  old.dmacon = custom->dmaconr;
  old.intena = custom->intenar;
  old.intreq = custom->intreqr;

  /* Prohibit dma & interrupts. */
  custom->adkcon = (UWORD)~ADKF_SETCLR;
  custom->dmacon = (UWORD)~DMAF_SETCLR;
  custom->intena = (UWORD)~INTF_SETCLR;
  WaitVBlank();

  /* Clear all interrupt requests. Really. */
  custom->intreq = (UWORD)~INTF_SETCLR;
  custom->intreq = (UWORD)~INTF_SETCLR;

  /* CIA-A & CIA-B: Mask all interrupts, get old masks. */
  old.icrCiaA = AbleICR(old.resCiaA, CIAICRF_ALL);
  old.icrCiaB = AbleICR(old.resCiaB, CIAICRF_ALL);

  /* CIA-A & CIA-B: Save state of all timers. */
  GetTimerState(&ciaa->ciacra, &ciaa->ciatalo, &ciaa->ciatahi, &old.timer[0]);
  GetTimerState(&ciaa->ciacrb, &ciaa->ciatblo, &ciaa->ciatbhi, &old.timer[1]);
  GetTimerState(&ciab->ciacra, &ciab->ciatalo, &ciab->ciatahi, &old.timer[2]);
  GetTimerState(&ciab->ciacrb, &ciab->ciatblo, &ciab->ciatbhi, &old.timer[3]);

  /* CIA-A & CIA-B: Stop timers and return to default settings. */
  ciaa->ciacra = 0;
  ciaa->ciacrb = 0;
  ciab->ciacra = 0;
  ciab->ciacrb = 0;

  {
    struct Task *self = FindTask(NULL);
    bd->bd_stkbot = self->tc_SPLower;
    bd->bd_stksz = self->tc_SPUpper - self->tc_SPLower;
  }

  /* Enter supervisor mode and save exception vector
   * since the framework takes full control over it. */
  old.sysStack = SuperState();

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

  memcpy(old.excVec, bd->bd_vbr, sizeof(old.excVec));

  return &BootData;
}

extern void UserState34(void *stack);

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
  memcpy(bd->bd_vbr, old.excVec, sizeof(old.excVec));

  /* UserState is broken on V33/34 Kickstart, hangs on 68010. */
  if (ExecVer <= 34) {
    UserState34(old.sysStack);
  } else {
    UserState(old.sysStack);
  }

  /* CIA-A & CIA-B: Restore state of all timers. */
  SetTimerState(&ciaa->ciacra, &ciaa->ciatalo, &ciaa->ciatahi, &old.timer[0]);
  SetTimerState(&ciaa->ciacrb, &ciaa->ciatblo, &ciaa->ciatbhi, &old.timer[1]);
  SetTimerState(&ciab->ciacra, &ciab->ciatalo, &ciab->ciatahi, &old.timer[2]);
  SetTimerState(&ciab->ciacrb, &ciab->ciatblo, &ciab->ciatbhi, &old.timer[3]);

  /* CIA-A & CIA-B: Restore old interrupts masks. */
  AbleICR(old.resCiaA, old.icrCiaA | CIAICRF_SETCLR);
  AbleICR(old.resCiaB, old.icrCiaB | CIAICRF_SETCLR);

  /* Restore AmigaOS state of dma & interrupts. */
  custom->intreq = old.intreq | INTF_SETCLR;
  custom->intena = old.intena | INTF_SETCLR;
  custom->dmacon = old.dmacon | DMAF_SETCLR;
  custom->adkcon = old.adkcon | ADKF_SETCLR;

  /* Restore old copper list... */
  custom->cop1lc = (ULONG)GfxBase->copinit;
  WaitTOF();

  /* ... and original view. */
  LoadView(old.view);
  WaitTOF();
  WaitTOF();

  /* Enable CPU caches. */
  if (ExecVer >= 36)
    CacheControl(old.cacheBits, -1);

  /* Restore multitasking. */
  Permit();

  /* Deallocate blitter. */
  DisownBlitter();

  CloseLibrary((struct Library *)GfxBase);
}
