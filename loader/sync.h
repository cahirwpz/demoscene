#ifndef __SYNC_H__
#define __SYNC_H__

#include "common.h"

typedef struct Track TrackT;

extern TrackT **__TRACK_LIST__;

void InitTracks(void);

__regargs TrackT *TrackLookup(const char *name);
__regargs short TrackValueGet(TrackT *track, short frame);

#endif
