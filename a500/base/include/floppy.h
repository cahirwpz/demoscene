#ifndef __FLOPPY_H__
#define __FLOPPY_H__

#include "common.h"

#define TD_SECTOR 512
#define NUMSECS 11

void InitFloppy();
void KillFloppy();

__regargs void FloppyTrackRead(WORD num);
__regargs void FloppyTrackDecode(ULONG *buf);

#endif
