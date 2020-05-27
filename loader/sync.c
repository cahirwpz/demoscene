#include "sync.h"
#include "fx.h"

/* Introduce weak symbol in case no tracks were defined by user. */
TrackT *__TRACK_LIST__[1];

static __regargs void TrackAdvance(TrackT *track, TrackKeyT *curr) {
  TrackKeyT *next;

  if (curr->frame == CTRL_KEY) {
    track->type = curr->value;
    curr++;
  }
  track->curr = curr;

  next = curr + 1;
  if (next->frame == CTRL_KEY)
    next++;
  track->next = next;

  if (next->frame != END_KEY) {
    track->interval = next->frame - curr->frame;
    track->delta = next->value - curr->value;
    track->pending = true;
  }
}

__regargs void TrackReset(TrackT *track) {
  track->type = TRACK_LINEAR;
  track->pending = true;

  TrackAdvance(track, track->data);
}

void InitTracks(void) {
  TrackT **tracks = __TRACK_LIST__;
  TrackT *track;

  while ((track = *tracks++)) {
    Log("[Sync] Initializing track '%s'\n", track->name);
    TrackReset(track);
  }
}

__regargs short TrackValueGet(TrackT *track, short frame) {
  TrackKeyT *curr = track->curr;
  TrackKeyT *next = track->next;
  short step;

  if (frame < curr->frame) {
    if ((track->type != TRACK_TRIGGER) &&
        (track->type != TRACK_EVENT))
      return curr->value;
    return 0;
  }

  if (next->frame == END_KEY) {
    if ((track->type != TRACK_TRIGGER) &&
        (track->type != TRACK_EVENT))
      return curr->value;
  } else {
    /* need to advance to next frame span? */
    while (frame >= next->frame) {
      TrackAdvance(track, curr + 1);

      curr = track->curr;
      next = track->next;

      if (next->frame == END_KEY)
        return curr->value;
    }
  }

  step = frame - curr->frame;

  switch (track->type) {
    case TRACK_RAMP:
      return curr->value;

    case TRACK_LINEAR:
      return curr->value + div16(step * track->delta, track->interval);

    case TRACK_SMOOTH:
      {
        short t = div16(shift12(step) / 2, track->interval);
        short k = (fx12f(1.0) - sintab[t + SIN_HALF_PI]) / 2;
        return curr->value + normfx(track->delta * k);
      }

    case TRACK_TRIGGER:
      {
        short v = curr->value - step;
        return (v > 0) ? v : 0;
      }

    case TRACK_EVENT:
      if (!track->pending)
        return 0;
      track->pending = false;
      return curr->value;

    default:
      return 0;
  }
}

__regargs TrackT *TrackLookup(const char *name) {
  TrackT **tracks = __TRACK_LIST__;
  do {
    TrackT *track = *tracks++;

    if (!strcmp(track->name, name))
      return track;
  } while (*tracks);

  return NULL;
}
