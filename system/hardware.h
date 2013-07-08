#ifndef __SYSTEM_HARDWARE_H__
#define __SYSTEM_HARDWARE_H__

#include <hardware/cia.h>
#include <hardware/custom.h>
#include <hardware/dmabits.h>
#include <hardware/intbits.h>

#define INTF_LEVEL3 (INTF_VERTB | INTF_BLIT | INTF_COPER)
#define INTF_LEVEL4 (INTF_AUD0 | INTF_AUD1 | INTF_AUD2 | INTF_AUD3)

extern volatile struct Custom* const custom;
extern volatile struct CIA* const ciaa;
extern volatile struct CIA* const ciab;

#endif
