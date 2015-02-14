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
#define BPLCON0_HOMOD   (1 << 11)
#define BPLCON0_HIRES   (1 << 15)
#define BPLCON2_PF2P2   (1 << 5)
#define BPLCON2_PF1P2   (1 << 2)

extern volatile struct Custom* const custom;
extern volatile struct CIA* const ciaa;
extern volatile struct CIA* const ciab;

void WaitMouse();
__regargs void WaitLine(ULONG line);
static void inline WaitVBlank() { WaitLine(303); }

static inline BOOL LeftMouseButton() {
  return !(ciaa->ciapra & CIAF_GAMEPORT0);
}

__regargs void Wait280ns(ULONG delay);

LONG ReadLineCounter();
LONG ReadFrameCounter();
void SetFrameCounter(ULONG frame);

#endif
