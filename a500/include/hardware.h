#ifndef __HARDWARE_H__
#define __HARDWARE_H__

#include <hardware/adkbits.h>
#include <hardware/cia.h>
#include <hardware/custom.h>
#include <hardware/dmabits.h>
#include <hardware/intbits.h>
#include <hardware/blit.h>

#include "common.h"

#define INTF_LEVEL1 (INTF_SOFTINT | INTF_DSKBLK | INTF_TBE)
#define INTF_LEVEL3 (INTF_VERTB | INTF_BLIT | INTF_COPER)
#define INTF_LEVEL4 (INTF_AUD0 | INTF_AUD1 | INTF_AUD2 | INTF_AUD3)
#define INTF_LEVEL5 (INTF_DSKSYNC | INTF_RBF)

#define BPLCON0_BPU(d)  (((d) & 7) << 12)
#define BPLCON0_COLOR   (1 << 9)
#define BPLCON0_LACE    (1 << 2)
#define BPLCON0_DBLPF   (1 << 10)
#define BPLCON0_HOMOD   (1 << 11)
#define BPLCON0_HIRES   (1 << 15)
#define BPLCON2_PF2PRI  (1 << 6)
#define BPLCON2_PF2P2   (1 << 5)
#define BPLCON2_PF1P2   (1 << 2)

#define DSK_DMAEN (1 << 15)
#define DSK_WRITE (1 << 14)
#define DSK_SYNC 0x4489

#define SERDATR_RBF  (1 << 14)
#define SERDATR_TBE  (1 << 13)
#define SERDATR_TSRE (1 << 12) 

extern volatile struct Custom* const custom;
extern volatile struct CIA* const ciaa;
extern volatile struct CIA* const ciab;

void WaitMouse();
__regargs void WaitLine(ULONG line);
static void inline WaitVBlank() { WaitLine(303); }

#define MoveLong(reg, hi, lo) \
    *(ULONG *)(&custom->reg) = (((hi) << 16) | (lo))

static inline BOOL LeftMouseButton() {
  return !(ciaa->ciapra & CIAF_GAMEPORT0);
}

static inline BOOL RightMouseButton() {
  return !(custom->potinp & (1 << 10));
}

#define E_CLOCK 0.709379
#define TIMER_MS(ms) ((ms) * 1000 * E_CLOCK)
#define TIMER_US(ms) ((ms) * E_CLOCK)

/* Maximum delay is around 92.38ms */
__regargs void WaitTimerA(volatile struct CIA *cia, UWORD delay);
__regargs void WaitTimerB(volatile struct CIA *cia, UWORD delay);
__regargs LONG ReadICR(volatile struct CIA *cia);

LONG ReadLineCounter();
LONG ReadFrameCounter();
void SetFrameCounter(ULONG frame);

#endif
