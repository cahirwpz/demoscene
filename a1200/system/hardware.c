#include "system/hardware.h"

volatile struct Custom* const custom = (APTR)0xdff000;
volatile struct CIA* const ciaa = (APTR)0xbfe001;
volatile struct CIA* const ciab = (APTR)0xbfd000;
