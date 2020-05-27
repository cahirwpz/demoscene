#include "sync.h"
#include "fx.h"

/* Introduce weak symbol in case no tracks were defined by user. */
TrackT **__TRACK_LIST__;

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

struct Track {
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
  char *name;
  TrackKeyT data[0];
};

static __regargs void TrackInit(TrackT *track) {
  TrackKeyT *key = track->data;
  TrackKeyT *next;

  track->type = TRACK_LINEAR;
  track->pending = true;

  while (key->frame == CTRL_KEY) {
    track->type = key->value;
    key++;
  }
  track->key[0] = track->key[1] = key;

  next = key + 1;
  while (next->frame == CTRL_KEY)
    next++;
  track->key[2] = next;

  if (next->frame != END_KEY) {
    track->interval = next->frame - key->frame;
    track->delta = next->value - key->value;

    do { next++; } while (next->frame == CTRL_KEY);

    if (next->frame == END_KEY)
      next = track->key[2];

    track->key[3] = next;
  }
}

void InitTracks(void) {
  TrackT *track;
  TrackT **tracks = __TRACK_LIST__;
  Log("[Sync] Initializing sync tracks\n");
  for (track = *tracks; track; tracks++)
    TrackInit(track);
}

/*
 * h00(t) := 2 * t^3 - 3 * t^2 + 1
 * h10(t) := t^3 - 2 * t^2 + t
 * h01(t) := -2 * t^3 + 3 * t^2
 * h11(t) := t^3 - t^2
 * p(t) := h00(t) * p(0) + h10(t) * m(0) + h01(t) * p(1) + h11(t) * m(1)
 *
 * m(k) := (p(k+1) - p(k-1)) / (t(k+1) - t(k-1))
 *
 * h11(t) := (t - 1) * t^2
 * ha(t) := h11(t) - t^2
 * hb(t) := h11(t) + ha(t)
 * h00(t) := hb(t) + 1
 * h10(t) := ha(t) + t
 * h01(t) := -hb(t)
 */
static __regargs int HermiteInterpolator(short t, TrackKeyT *key[4]) {
  register short one asm("d6") = fx12f(1.0);
  short t2 = normfx(t * t);
  short h11 = normfx(t2 * (short)(t - one));
  short ha = h11 - t2;
  short hb = h11 + ha;
  short h00 = hb + one;
  short h10 = ha + t;
  short h01 = -hb;

  short v0 = (*key++)->value;
  short v1 = (*key++)->value;
  short v2 = (*key++)->value;
  short v3 = (*key++)->value;

  return h00 * v1 + h01 * v2 +
    ((h10 * (short)(v2 - v0) + h11 * (short)(v3 - v1)) >> 1);
}

__regargs short TrackValueGet(TrackT *track, short frame) {
  TrackKeyT *key = track->key[1]; 
  TrackKeyT *next = track->key[2];
  short step;

  if (frame < key->frame) {
    if ((track->type != TRACK_TRIGGER) &&
        (track->type != TRACK_EVENT))
      return key->value;
    return 0;
  }

  if (next->frame == END_KEY) {
    if ((track->type != TRACK_TRIGGER) &&
        (track->type != TRACK_EVENT))
      return key->value;
  } else {
    /* need to advance to next frame span? */
    while (frame >= next->frame) {
      key++;

      while (key->frame == CTRL_KEY) {
        track->type = key->value;
        key++;
      }

      do { next++; } while (next->frame == CTRL_KEY);

      track->key[1] = key;
      track->key[2] = next;

      if (next->frame == END_KEY)
        return key->value;

      track->interval = next->frame - key->frame;
      track->delta = next->value - key->value;
      track->pending = true;

      if (track->type == TRACK_SPLINE) {
        do { next++; } while (next->frame == CTRL_KEY);

        if (next->frame == END_KEY)
          next = track->key[2];

        track->key[3] = next;
      }
    }
  }

  step = frame - key->frame;

  switch (track->type) {
    case TRACK_RAMP:
      return key->value;

    case TRACK_LINEAR:
      return key->value + div16(step * track->delta, track->interval);

    case TRACK_SMOOTH:
      {
        short t = div16(shift12(step) / 2, track->interval);
        short k = (fx12f(1.0) - sintab[t + SIN_HALF_PI]) / 2;
        return key->value + normfx(track->delta * k);
      }

    case TRACK_SPLINE:
      {
        short t = div16(shift12(step), track->interval);
        return normfx(HermiteInterpolator(t, track->key));
      }

    case TRACK_TRIGGER:
      {
        short v = key->value - step;
        return (v > 0) ? v : 0;
      }

    case TRACK_EVENT:
      if (!track->pending)
        return 0;
      track->pending = false;
      return key->value;

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
