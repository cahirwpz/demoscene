#include "sound.h"
#include "iff.h"
#include "memory.h"

#define ID_8SVX MAKE_ID('8', 'S', 'V', 'X')
#define ID_VHDR MAKE_ID('V', 'H', 'D', 'R')
#define ID_CHAN MAKE_ID('C', 'H', 'A', 'N')

#define LEFT   2L
#define RIGHT  4L
#define STEREO 6L

typedef struct VoiceHeader {
  u_int oneShotHiSamples;  /* # samples in the high octave 1-shot part */
  u_int repeatHiSamples;   /* # samples in the high octave repeat part */
  u_int samplesPerHiCycle; /* # samples/cycle in high octave, else 0   */
  u_short samplesPerSec;     /* data sampling rate                       */
  u_char ctOctave;          /* # octaves of waveforms                   */
  u_char sCompression;      /* data compression technique used          */
  u_int volume;            /* playback volume from 0 to 65536 (fp16)   */
} VoiceHeaderT;

__regargs SoundT *Load8SVX(const char *filename) {
  SoundT *sound = NULL;
  IffFileT iff;

  OpenIff(&iff, filename);

  if (iff.header.type != ID_8SVX)
    Panic("[8SVX] File '%s' has wrong type!\n", filename);

  while (ParseChunk(&iff)) {
    if (iff.chunk.type == ID_VHDR) {
      VoiceHeaderT vhdr;

      ReadChunk(&iff, &vhdr);
      sound = NewSound(0, vhdr.samplesPerSec);
      sound->volume = (vhdr.volume - 1) >> 10;
    } else if (iff.chunk.type == ID_BODY) {
      sound->length = iff.chunk.length;
      sound->sample = MemAlloc(iff.chunk.length, MEMF_CHIP);
      ReadChunk(&iff, sound->sample);
    } else {
      SkipChunk(&iff);
    }
  }

  CloseIff(&iff);

  return sound;
}
