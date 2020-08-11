#ifndef __FLOPPY_H__
#define __FLOPPY_H__

#include "common.h"

#define TRACK_SIZE 12800
#define TD_SECTOR 512
#define NSECTORS 11
#define NTRACKS 160

void InitFloppy(void);
void KillFloppy(void);

void FloppyTrackRead(short num);
void FloppyTrackDecode(u_int *buf);

#endif
