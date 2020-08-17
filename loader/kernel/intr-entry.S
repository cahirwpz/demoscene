#include <asm.h>
#include <custom_regdef.h>
#include <interrupt.h>

.macro  BEGIN   intb
        movem.l d0-d1/a0-a1,-(sp)
        lea     IntVec+8*\intb,a0
        moveq   #\intb,d1
        move.w  custom+intreqr,d0
.endm

.macro  HANDLE  irq
        btst    d1,d0
        jne     _L(EnterIntr)
        addq.l  #8,a0
        addq.w  #1,d1
.endm

.macro  FINISH  irq
        jra     _L(EnterIntr)
.endm

# Level 1 Interrupt Autovector

ENTRY(AmigaLvl1Handler)
        BEGIN   INTB_TBE
        HANDLE  TBE
        HANDLE  DSKBLK
        FINISH  SOFTINT
END(AmigaLvl1Handler)

# Level 2 Interrupt Autovector

ENTRY(AmigaLvl2Handler)
        BEGIN   INTB_PORTS
        FINISH  PORTS
END(AmigaLvl2Handler)

# Level 3 Interrupt Autovector

ENTRY(AmigaLvl3Handler)
        BEGIN   INTB_COPER
        HANDLE  COPER
        HANDLE  VERTB
        FINISH  BLIT
END(AmigaLvl3Handler)

# Level 4 Interrupt Autovector

ENTRY(AmigaLvl4Handler)
        BEGIN   INTB_AUD0
        HANDLE  AUD0
        HANDLE  AUD1
        HANDLE  AUD2
        FINISH  AUD3
END(AmigaLvl4Handler)

# Main part of interrupt handler

ENTRY(EnterIntr)
        /* Make INTF_* mask, and clear pending interrupt. */
        clr.w   d0
        bset    d1,d0
        move.w  d0,custom+intreq

        /* Enter interrupt service routine. */
        move.l  (a0)+,a1                /* IntVecEntry_t.code */
        move.l  (a0)+,a0                /* IntVecEntry_t.data */
        jsr     (a1)
        movem.l (sp)+,d0-d1/a0-a1
        rte
END(EnterIntr)

# Level 5 Interrupt Autovector

ENTRY(AmigaLvl5Handler)
        BEGIN   INTB_RBF
        HANDLE  RBF
        FINISH  DSKSYNC
END(AmigaLvl5Handler)

# Level 6 Interrupt Autovector

ENTRY(AmigaLvl6Handler)
        BEGIN   INTB_EXTER
        FINISH  EXTER
END(AmigaLvl6Handler)

# Dummy handler

ENTRY(DummyInterruptHandler)
        rts
END(DummyInterruptHandler)

# vim: ft=gas:ts=8:sw=8:noet:
