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
  ULONG oneShotHiSamples;  /* # samples in the high octave 1-shot part */
  ULONG repeatHiSamples;   /* # samples in the high octave repeat part */
  ULONG samplesPerHiCycle; /* # samples/cycle in high octave, else 0   */
  UWORD samplesPerSec;     /* data sampling rate                       */
  UBYTE ctOctave;          /* # octaves of waveforms                   */
  UBYTE sCompression;      /* data compression technique used          */
  ULONG volume;            /* playback volume from 0 to 65536 (fp16)   */
} VoiceHeaderT;

__regargs SoundT *Load8SVX(const char *filename) {
  SoundT *sound = NULL;
  IffFileT iff;

  if (OpenIff(&iff, filename)) {
    if (iff.header.type == ID_8SVX) {
      while (ParseChunk(&iff)) {
        switch (iff.chunk.type) {
          case ID_VHDR:
            {
              VoiceHeaderT vhdr;

              ReadChunk(&iff, &vhdr);
              sound = NewSound(0, vhdr.samplesPerSec);
              sound->volume = (vhdr.volume - 1) >> 10;
            }
            break;

          case ID_BODY:
            sound->length = iff.chunk.length;
            sound->sample = MemAlloc(iff.chunk.length, MEMF_CHIP);
            ReadChunk(&iff, sound->sample);
            break;

          default:
            SkipChunk(&iff);
            break;
        }
      }
    }

    CloseIff(&iff);
  }

  return sound;
}
