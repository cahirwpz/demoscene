#include "sync.h"
#include "fx.h"

__regargs void TrackInit(TrackT *track) {
  TrackKeyT *key = track->data;
  TrackKeyT *next;

  track->type = TRACK_LINEAR;

  while (key->frame == CTRL_KEY) {
    track->type = key->value;
    key++;
  }
  track->key = key;

  next = key + 1;
  while (next->frame == CTRL_KEY)
    next++;
  track->next = next;

  if (next->frame != END_KEY) {
    track->interval = next->frame - key->frame;
    track->delta = next->value - key->value;
  }
}

__regargs WORD TrackValueGet(TrackT *track, WORD frame) {
  TrackKeyT *key = track->key; 
  TrackKeyT *next = track->next;
  WORD step;

  if (frame < key->frame)
    return (track->type == TRACK_TRIGGER) ? 0 : key->value;

  if (next->frame == END_KEY) {
    if (track->type == TRACK_TRIGGER) {
      step = frame - key->frame;
      return (step < key->value) ? (key->value - step) : 0;
    }
    return key->value;
  }

  /* need to advance to next frame span? */
  while (frame >= next->frame) {
    key++; next++;

    while (key->frame == CTRL_KEY) {
      track->type = key->value;
      key++;
    }

    while (next->frame == CTRL_KEY)
      next++;

    track->key = key;
    track->next = next;

    if (next->frame == END_KEY)
      return key->value;

    track->interval = next->frame - key->frame;
    track->delta = next->value - key->value;
  }

  step = frame - key->frame;

  switch (track->type) {
    case TRACK_RAMP:
       return key->value;

    case TRACK_TRIGGER:
       return (step < key->value) ? (key->value - step) : 0;

    case TRACK_LINEAR:
      return key->value + div16(step * track->delta, track->interval);

    case TRACK_SMOOTH:
      {
        WORD t = div16(shift12(step) / 2, track->interval);
        WORD k = (fx12f(1.0) - sintab[t + SIN_HALF_PI]) / 2;
        return key->value + normfx(track->delta * k);
      }
  }

  return 0;
}

__regargs TrackT *TrackLookup(TrackT **tracks, const char *name) {
  do {
    TrackT *track = *tracks++;

    if (!strcmp(track->name, name))
      return track;
  } while (*tracks);

  return NULL;
}
