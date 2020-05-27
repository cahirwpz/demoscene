#ifndef __SYNC_H__
#define __SYNC_H__

#include "common.h"

typedef enum {
  TRACK_RAMP    = 1, /* set constant value */
  TRACK_LINEAR  = 2, /* lerp to the next value */
  TRACK_SMOOTH  = 3, /* smooth curve to the next value */
  TRACK_SPLINE  = 4,
  TRACK_TRIGGER = 5, /* count down (with every frame) from given number */
  TRACK_EVENT   = 6  /* like ramp but value is delivered only once */
} __attribute__((packed)) TrackTypeT;

#define END_KEY  -1
#define CTRL_KEY -2

typedef struct {
  short frame;
  short value;
} TrackKeyT;

/*
 * A few simplifying assumptions:
 * (1) There's at least one data key in the track.
 * (2) Default track's type is TYPE_LINEAR.
 * (3) There's always a data key before the end key.
 */

typedef struct Track {
  /* private */
  TrackTypeT type;
  /*
   * 0 => previous,
   * 1 => current (always points to data)
   * 2 => next (always points to data or end key)
   * 3 => next + 1
   */
  TrackKeyT *key[4]; 
  short interval;
  short delta;
  bool pending;
  /* public: provided by user */
  const char *name;
  TrackKeyT data[0];
} TrackT;

extern TrackT *__TRACK_LIST__[];

void InitTracks(void);

__regargs TrackT *TrackLookup(const char *name);
__regargs short TrackValueGet(TrackT *track, short frame);

#endif
