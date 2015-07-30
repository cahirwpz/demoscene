#include "memory.h"
#include "io.h"
#include "reader.h"
#include "sync.h"

static __regargs TrackT *ReadTrack(char **strptr) {
  TrackT *track = NULL;
  char *data;
  WORD pass;

  for (pass = 0; pass < 2; pass++) {
    WORD keys = 0;
    WORD last_pos = -1;
    WORD name_len = 0;
    BOOL end = FALSE;

    data = *strptr;

    while (*data && !end) {
      TrackTypeT type;
      WORD pos = -1, value;

      data = SkipSpaces(data);

      switch (*data) {
        case '#':
          data = NextLine(data);
          break;

        case '@':
          data++;
          if (!memcmp(data, "track", 5)) {
            char *name = NULL;
            data += 5;
            name_len = ReadSymbol(&data, &name);
            if (pass) {
              char *src = name;
              char *dst = track->name;
              WORD n = name_len;
              while (--n >= 0)
                *dst++ = *src++;
            }
          } else if (!memcmp(data, "end", 3)) {
            end = TRUE;
            data += 3;
          } else {
            Print("Unknown directive!\n");
            goto quit;
          }
          break;

        case '!':
          data++;
          if (!memcmp(data, "ramp", 4)) {
            type = TRACK_RAMP;
            data += 4;
          } else if (!memcmp(data, "linear", 6)) {
            type = TRACK_LINEAR;
            data += 6;
          } else if (!memcmp(data, "smooth", 6)) {
            type = TRACK_SMOOTH;
            data += 6;
          } else if (!memcmp(data, "spline", 6)) {
            type = TRACK_SPLINE;
            data += 6;
          } else if (!memcmp(data, "trigger", 7)) {
            type = TRACK_TRIGGER;
            data += 7;
          } else if (!memcmp(data, "event", 5)) {
            type = TRACK_EVENT;
            data += 5;
          } else {
            Print("Unknown control key!\n");
            goto quit;
          }

          if (pass) {
            track->data[keys].frame = CTRL_KEY;
            track->data[keys].value = type;
          }
          keys++;
          break;

        case '$':
          if (!ReadNumber(&data, &pos) || (pos < 0)) {
            Print("Module position is an invalid number!\n");
            goto quit;
          }
          if (last_pos >= pos) {
            Print("Frame number does not grow monotonically : %lx -> %lx!\n",
                (LONG)last_pos, (LONG)pos);
            goto quit;
          }
          last_pos = pos;
          if (!ReadNumber(&data, &value)) {
            Print("Value is not a number!\n");
            goto quit;
          }

          if (pass) {
            track->data[keys].frame = MOD_POS_FRAME(pos);
            track->data[keys].value = value;
          }
          keys++;
          break;

        default:
          Print("Syntax error!\n");
          goto quit;
      }
    }

    if (!pass) {
      WORD size = sizeof(TrackT) + sizeof(TrackKeyT) * (keys + 1);
      track = MemAllocAuto(size + name_len + 1, MEMF_PUBLIC|MEMF_CLEAR);
      track->name = (char *)track + size;
    } else {
      track->data[keys].frame = END_KEY;
      TrackInit(track);
    }
  }

quit:
  *strptr = SkipSpaces(data);
  return track;
}

__regargs TrackT *LoadTrack(char *filename) {
  char *file = LoadFile(filename, MEMF_PUBLIC);
  TrackT *track = NULL;

  if (file) {
    char *data = file;

    if (!(track = ReadTrack(&data)))
      Print("Error at byte %ld!\n", (LONG)(data - file));

    MemFreeAuto(file);
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
        Print("Error at byte %ld!\n", (LONG)(data - file));
        while (count)
          MemFreeAuto(tmp[--count]);
        break;
      }

      Log("Track '%s' loaded.\n", track->name);

      tmp[count] = track;
    }

    if (count > 0) {
      tracks = MemAllocAuto(sizeof(TrackT *) * (count + 1), MEMF_PUBLIC);
      memcpy(tracks, tmp, sizeof(TrackT *) * count);
      tracks[count] = NULL;
    }

    MemFreeAuto(file);
  }

  return tracks;
}

__regargs void DeleteTrackList(TrackT **tracks) {
  TrackT **tmp = tracks;
  do { MemFreeAuto(*tmp++); } while (*tmp);
  MemFreeAuto(tracks);
}
