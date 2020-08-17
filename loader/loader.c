#include <proto/alib.h>
#include <proto/exec.h>
#include <exec/execbase.h>
#include <exec/memory.h>

#include <debug.h>
#include <cpu.h>
#include <custom.h>
#include <interrupt.h>
#include <exception.h>
#include <trap.h>
#include <memory.h>
#include <io.h>

#include "autoinit.h"
#include "sync.h"

static struct { short version; short revision; } kickstart;

u_char CpuModel = CPU_68000;
u_char FpuModel = FPU_NONE;

extern int main(void);

#define THRESHOLD 4096

static void InitMemory(void) {
  struct List *list = &SysBase->MemList;
  struct Node *node;

  for (node = list->lh_Head; node->ln_Succ; node = node->ln_Succ) {
    struct MemHeader *mh = (struct MemHeader *)node;

    if (mh->mh_Attributes & MEMF_PUBLIC) {
      struct MemChunk *mc;
     
      for (mc = mh->mh_First; mc; mc = mc->mc_Next) {
        if (mc->mc_Bytes >= THRESHOLD)
          AddMemory((void *)mc + sizeof(struct MemChunk), mc->mc_Bytes,
                    mh->mh_Attributes);
      }
    }
  }
}

/* Exception Vector Base: 0 for 68000, for 68010 and above read from VBR */
ExcVecT *ExcVecBase = (ExcVecT *)NULL;

/* Amiga autovector interrupts table. */
IntVecT IntVec;

static void SetupExceptionVector(void) {
  short i;

  if (CpuModel >= CPU_68010)
    ExcVecBase = GetVBR();

  /* Initialize M68k interrupt vector. */
  for (i = EXC_BUSERR; i <= EXC_LAST; i++)
    ExcVec[i] = BadTrap;

  /* Initialize exception handlers. */
  ExcVec[EXC_BUSERR] = BusErrTrap;
  ExcVec[EXC_ADDRERR] = AddrErrTrap;
  ExcVec[EXC_ILLEGAL] = IllegalTrap;
  ExcVec[EXC_ZERODIV] = ZeroDivTrap;
  ExcVec[EXC_CHK] = ChkInstTrap;
  ExcVec[EXC_TRAPV] = TrapvInstTrap;
  ExcVec[EXC_PRIV] = PrivInstTrap;
  ExcVec[EXC_TRACE] = TraceTrap;
  ExcVec[EXC_LINEA] = IllegalTrap;
  ExcVec[EXC_LINEF] = IllegalTrap;
  ExcVec[EXC_FMTERR] = FmtErrTrap;

  /* Initialize level 1-7 interrupt autovector in Amiga specific way. */
  ExcVec[EXC_INTLVL(1)] = AmigaLvl1Handler;
  ExcVec[EXC_INTLVL(2)] = AmigaLvl2Handler;
  ExcVec[EXC_INTLVL(3)] = AmigaLvl3Handler;
  ExcVec[EXC_INTLVL(4)] = AmigaLvl4Handler;
  ExcVec[EXC_INTLVL(5)] = AmigaLvl5Handler;
  ExcVec[EXC_INTLVL(6)] = AmigaLvl6Handler;

  for (i = INTB_TBE; i <= INTB_EXTER; i++)
    IntVec[i].code = DummyInterruptHandler;

  /* Intialize TRAP instruction handlers. */
  for (i = EXC_TRAP(0); i <= EXC_TRAP(15); i++)
    ExcVec[i] = TrapInstTrap;
}

static void ReadCpuModel(void) {
  if (SysBase->AttnFlags & AFF_68060)
    CpuModel = CPU_68060;
  else if (SysBase->AttnFlags & AFF_68040)
    CpuModel = CPU_68040;
  else if (SysBase->AttnFlags & AFF_68030)
    CpuModel = CPU_68030;
  else if (SysBase->AttnFlags & AFF_68020)
    CpuModel = CPU_68020;
  else if (SysBase->AttnFlags & AFF_68010)
    CpuModel = CPU_68010;
}

static void ReadKickVersion(void) {
  /* Based on WhichAmiga method. */
  void *kickEnd = (void *)0x1000000;
  u_int kickSize = *(u_int *)(kickEnd - 0x14);
  u_short *kick = kickEnd - kickSize;

  kickstart.version = kick[6];
  kickstart.revision = kick[7];
}

static void SetupProcessor(void) {
  /* Disable CPU caches. */
  if (kickstart.version >= 36)
    (void)CacheControl(0, -1);

  /* Enter supervisor state. */
  (void)SuperState();
}

static void SetupCustomChips(void) {
  /* Prohibit DMA & interrupts. */
  custom->adkcon = (u_short)~ADKF_SETCLR;
  DisableDMA(DMAF_ALL);
  DisableINT(INTF_ALL);
  WaitVBlank();

  /* Clear all interrupt requests. Really! */
  ClearIRQ(INTF_ALL);
  ClearIRQ(INTF_ALL);

  /* Enable master switches. */
  EnableDMA(DMAF_MASTER);
  EnableINT(INTF_INTEN);
}

void Loader(void) {
  /* No calls to any other library than exec beyond this point or expect
   * undefined behaviour including crashes. */
  Forbid();

  ReadCpuModel();
  ReadKickVersion();

  Log("[Loader] ROM: %d.%d, CPU: 680%d0, CHIP: %ldkB, FAST: %ldkB\n",
      kickstart.version, kickstart.revision, CpuModel,
      AvailMem(MEMF_CHIP | MEMF_LARGEST) / 1024,
      AvailMem(MEMF_FAST | MEMF_LARGEST) / 1024);

  SetupProcessor();
  SetupExceptionVector();
  SetupCustomChips();
  InitMemory();
  InitFloppyIO();
  InitTracks();
  CallFuncList(&__INIT_LIST__);

  {
    int retval = main();
    Log("[Loader] main() returned %d.\n", retval);
  }

  CallFuncList(&__EXIT_LIST__);
  KillFloppyIO();
  
  Log("[Loader] Shutdown complete!\n")
}
