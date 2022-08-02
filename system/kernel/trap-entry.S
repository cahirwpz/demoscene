#include <asm.h>
#include <system/trap.h>

ENTRY(EnterTrap)
        movem.l d0-a7,-(sp)     /* save all registers */
        move.l  usp,a0
        move.l  a0,-(sp)        /* save user stack pointer */
        move.l  sp,a0           /* pass trap frame as an argument */
        jsr     _L(TrapHandler)
        move.l  (sp)+,a0
        move.l  a0,usp          /* restore user stack pointer */
        movem.l (sp)+,d0-a7     /* restore all registers */
        addq.l  #2,sp           /* pop trap number */
        rte
END(EnterTrap)

# List of trap handlers

.macro  TRAP    num
        move.w  #\num,-(sp)     /* save trap number */
        jra     _L(EnterTrap)
.endm

#define HANDLER(name,trap) ENTRY(name); TRAP trap; END(name)

HANDLER(BadTrap,T_UNKNOWN)
HANDLER(BusErrTrap,T_BUSERR)
HANDLER(AddrErrTrap,T_ADDRERR)
HANDLER(IllegalTrap,T_ILLINST)
HANDLER(ZeroDivTrap,T_ZERODIV)
HANDLER(ChkInstTrap,T_CHKINST)
HANDLER(TrapvInstTrap,T_TRAPVINST)
HANDLER(PrivInstTrap,T_PRIVINST)
HANDLER(TraceTrap,T_TRACE)
HANDLER(FmtErrTrap,T_FMTERR)
HANDLER(TrapInstTrap,T_TRAPINST)

# vim: ft=gas:ts=8:sw=8:noet:
