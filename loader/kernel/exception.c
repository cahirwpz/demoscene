#include <cpu.h>
#include <interrupt.h>
#include <exception.h>
#include <trap.h>

/* Exception Vector Base: 0 for 68000, for 68010 and above read from VBR */
ExcVecT *ExcVecBase = (ExcVecT *)NULL;

/* Amiga autovector interrupts table. */
IntVecT IntVec;

void SetupExceptionVector(void) {
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

  /* Initialize PORTS & VERTB & EXTER as interrupt server chain. */
  SetIntVector(PORTS, (IntHandlerT)RunIntChain, PortsChain);
  SetIntVector(VERTB, (IntHandlerT)RunIntChain, VertBlankChain);
  SetIntVector(EXTER, (IntHandlerT)RunIntChain, ExterChain);

  /* Intialize TRAP instruction handlers. */
  for (i = EXC_TRAP(0); i <= EXC_TRAP(15); i++)
    ExcVec[i] = TrapInstTrap;
}
