#ifndef __SYNC_H__
#define __SYNC_H__

#include "common.h"

#ifndef FRAMES_PER_ROW
#error "FRAMES_PER_ROW: not defined!"
#endif

#define MOD_POS_FRAME(pos) \
  (((((pos) >> 8) & 0xff) * 64 + ((pos) & 0x3f)) * FRAMES_PER_ROW)

#define TRK_INIT(...) { TRACK_LINEAR, NULL, NULL, 0, 0, NULL, {__VA_ARGS__} }
#define TRK_TYPE(type) { -2, TRACK_ ## type }
#define TRK_KEY(pos, value) { MOD_POS_FRAME(pos), value }
#define TRK_KEY_BIAS(pos, bias, value) { MOD_POS_FRAME(pos) + (bias), value }
#define TRK_END { -1, 0 }

typedef enum {
  TRACK_RAMP    = 1, /* set constant value */
  TRACK_TRIGGER = 2, /* count down from specific number of frames */
  TRACK_LINEAR  = 3, /* lerp to the next value */
  TRACK_SMOOTH  = 4, /* smooth curve to the next value */
} TrackTypeT;

#define END_KEY  -1
#define CTRL_KEY -2

typedef struct {
  WORD frame;
  WORD value;
} TrackKeyT;

/*
 * A few simplifying assumptions:
 * (1) There's at least one data key in the track.
 * (2) Default track's type is TYPE_LINEAR.
 * (3) There's always a data key before the end key.
 */

typedef struct {
  /* private */
  TrackTypeT type;
  TrackKeyT *key;  /* always points to data */
  TrackKeyT *next; /* always points to data or end key */
  WORD interval;
  WORD delta;
  /* public: provided by user */
  char *name;
  TrackKeyT data[0];
} TrackT;

__regargs void TrackInit(TrackT *track);
__regargs WORD TrackValueGet(TrackT *track, WORD frame);
__regargs TrackT *TrackLookup(TrackT **tracks, const char *name);

__regargs TrackT *LoadTrack(char *filename);
__regargs TrackT **LoadTrackList(char *filename);
__regargs void DeleteTrackList(TrackT **tracks);

#endif
