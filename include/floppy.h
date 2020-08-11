#ifndef __FLOPPY_H__
#define __FLOPPY_H__

#include "common.h"

#define TD_SECTOR 512
#define NUMSECS 11

void InitFloppy(void);
void KillFloppy(void);

void FloppyTrackRead(short num);
void FloppyTrackDecode(u_int *buf);

#endif
