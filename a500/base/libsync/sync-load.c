#include "memory.h"
#include "io.h"
#include "reader.h"
#include "sync.h"

static WORD ParseTrack(char **data, TrackT *track) {
  WORD keys = 0;
  WORD last_pos = -1;
  BOOL done = FALSE;

  while (NextLine(data) && !done) {
    TrackTypeT type = 0;
    WORD pos = -1, value = 0;

    if (MatchString(data, "@end")) {
      done = TRUE;
    } else if (MatchString(data, "!ramp")) {
      type = TRACK_RAMP;
    } else if (MatchString(data, "!linear")) {
      type = TRACK_LINEAR;
    } else if (MatchString(data, "!smooth")) {
      type = TRACK_SMOOTH;
    } else if (MatchString(data, "!spline")) {
      type = TRACK_SPLINE;
    } else if (MatchString(data, "!trigger")) {
      type = TRACK_TRIGGER;
    } else if (MatchString(data, "!event")) {
      type = TRACK_EVENT;
    } else if (ReadShort(data, &pos)) {
      if (pos < 0) {
        Log("Module position is negative!\n");
        return 0;
      }
      if (last_pos >= pos) {
        Log("Frame number does not grow monotonically : %lx -> %lx!\n",
            (LONG)last_pos, (LONG)pos);
        return 0;
      }
      if (!ReadShort(data, &value)) {
        Log("Number expected!\n");
        return 0;
      }
      last_pos = pos;
    } else {
      char c = **data;

      if (c == '!') {
        Log("Unknown control key!\n");
      } else if (c == '@') {
        Log("Unknown directive!\n");
      } else {
        Log("Syntax error!\n");
      }

      return 0;
    }

    if (!EndOfLine(data)) {
      Log("Cannot parse extra data!\n");
      return 0;
    }

    if (track) {
      if (type) {
        track->data[keys].frame = CTRL_KEY;
        track->data[keys].value = type;
      } else {
        track->data[keys].frame = MOD_POS_FRAME(pos);
        track->data[keys].value = value;
      }
    }

    keys++;
  }

  if (track) {
    track->data[keys].frame = END_KEY;
    TrackInit(track);
  }

  return keys;
}

static TrackT *ReadTrack(char **data) {
  char name[80];
  char *bogus;
  TrackT *track = NULL;
  WORD name_len, keys = 0;

  if (!(NextWord(data) && MatchString(data, "@track"))) {
    Log("No '@track' directive found\n");
    return NULL;
  }

  if (!((name_len = ReadString(data, name, sizeof(name))) && EndOfLine(data))) {
    Log("'@track' directive expects one argument\n");
    return NULL;
  }

  Log("[Track] Found '%s'.\n", name);

  bogus = *data;

  if ((keys = ParseTrack(&bogus, NULL))) {
    track = MemAlloc(sizeof(TrackT) +
                     sizeof(TrackKeyT) * (keys + 1) + 
                     name_len + 1, MEMF_PUBLIC|MEMF_CLEAR);
    track->name = (char *)&track->data[keys + 1];
    strcpy(track->name, name);
    /* BUG: Why this crashes? */
    /* memcpy(track->name, name, name_len); */
    ParseTrack(data, track);
  }

  return track;
}

#define MAX_TRACKS 128

__regargs TrackT **LoadTrackList(char *filename) {
  char *file = LoadFile(filename, MEMF_PUBLIC);
  TrackT **tracks = NULL;
  
  if (file) {
    TrackT **tmp = alloca(sizeof(TrackT *) * MAX_TRACKS);
    char *data = file;
    WORD count;

    for (count = 0; *data && (count < MAX_TRACKS); count++) {
      TrackT *track = ReadTrack(&data);

      if (!track) {
        Log("Error at byte %ld!\n", (LONG)(data - file));
        while (count)
          MemFree(tmp[--count]);
        break;
      }

      tmp[count] = track;
    }

    if (count > 0) {
      tracks = MemAlloc(sizeof(TrackT *) * (count + 1), MEMF_PUBLIC);
      memcpy(tracks, tmp, sizeof(TrackT *) * count);
      tracks[count] = NULL;
    }

    MemFree(file);
  }

  return tracks;
}

__regargs void DeleteTrackList(TrackT **tracks) {
  TrackT **tmp = tracks;
  do { MemFree(*tmp++); } while (*tmp);
  MemFree(tracks);
}
