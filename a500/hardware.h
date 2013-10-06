#ifndef __HARDWARE_H__
#define __HARDWARE_H__

#include <hardware/cia.h>
#include <hardware/custom.h>
#include <hardware/dmabits.h>
#include <hardware/intbits.h>

#include "common.h"

#define INTF_LEVEL3 (INTF_VERTB | INTF_BLIT | INTF_COPER)
#define INTF_LEVEL4 (INTF_AUD0 | INTF_AUD1 | INTF_AUD2 | INTF_AUD3)

#define BPLCON0_BPU(d)  (((d) & 7) << 12)
#define BPLCON0_COLOR   (1 << 9)

#define BLTCON0_USEA    (1 << 11)
#define BLTCON0_USEB    (1 << 10)
#define BLTCON0_USEC    (1 << 9)
#define BLTCON0_USED    (1 << 8)

#define BLTCON1_SIGN    (1 << 6)
#define BLTCON1_SUD     (1 << 4)  /* sometimes up or down */
#define BLTCON1_SUL     (1 << 3)  /* sometimes up or left */
#define BLTCON1_AUL     (1 << 2)  /* always up or left */
#define BLTCON1_ONEDOT  (1 << 1)
#define BLTCON1_LINE    (1 << 0)

extern volatile struct Custom* const custom;
extern volatile struct CIA* const ciaa;
extern volatile struct CIA* const ciab;

void WaitMouse();
void WaitBlitter();
__regargs void WaitLine(ULONG line);
static void inline WaitVBlank() { WaitLine(312); }

static inline BOOL LeftMouseButton() {
  return !(ciaa->ciapra & CIAF_GAMEPORT0);
}

#endif
