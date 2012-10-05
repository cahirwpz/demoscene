#ifndef __SYSTEM_TIMER_H__
#define __SYSTEM_TIMER_H__

#include "std/types.h"

bool SetupTimer();
void KillTimer();
uint64_t TimerRead();
double TimerDiff(uint64_t start, uint64_t end);

#endif
