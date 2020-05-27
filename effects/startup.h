#ifndef __STARTUP_H__
#define __STARTUP_H__

#include "types.h"

void SystemInfo(void);
void KillOS(void);
void RestoreOS(void);
void InitVBlank(void);
void KillVBlank(void);
void RunEffect(void);

#endif
