#ifndef __HARDWARE_H__
#define __HARDWARE_H__

#include <hardware/cia.h>
#include <hardware/custom.h>
#include <hardware/dmabits.h>
#include <hardware/intbits.h>
#include <hardware/blit.h>

#include "common.h"

#define INTF_LEVEL3 (INTF_VERTB | INTF_BLIT | INTF_COPER)
#define INTF_LEVEL4 (INTF_AUD0 | INTF_AUD1 | INTF_AUD2 | INTF_AUD3)

#define BPLCON0_BPU(d)  (((d) & 7) << 12)
#define BPLCON0_COLOR   (1 << 9)

extern volatile struct Custom* const custom;
extern volatile struct CIA* const ciaa;
extern volatile struct CIA* const ciab;

void WaitMouse();
__regargs void WaitLine(ULONG line);
static void inline WaitVBlank() { WaitLine(312); }

static inline BOOL LeftMouseButton() {
  return !(ciaa->ciapra & CIAF_GAMEPORT0);
}

#endif
