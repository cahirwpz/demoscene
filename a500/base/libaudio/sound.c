#include "sound.h"
#include "memory.h"

__regargs SoundT *NewSound(u_int length, u_short rate) {
  SoundT *sound = MemAlloc(sizeof(SoundT), MEMF_PUBLIC|MEMF_CLEAR);

  sound->length = length;
  sound->rate = rate;

  if (length)
    sound->sample = MemAlloc(sizeof(char) * length, MEMF_CHIP);

  return sound;
}

__regargs void DeleteSound(SoundT *sound) {
  if (sound) {
    MemFree(sound->sample);
    MemFree(sound);
  }
}
