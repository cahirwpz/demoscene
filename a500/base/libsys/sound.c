#include "hardware.h"
#include "sound.h"
#include "memory.h"

__regargs SoundT *NewSound(ULONG length, UWORD rate) {
  SoundT *sound = MemAlloc(sizeof(SoundT), MEMF_PUBLIC|MEMF_CLEAR);

  sound->length = length;
  sound->rate = rate;

  if (length)
    sound->sample = MemAlloc(sizeof(BYTE) * length, MEMF_CHIP);

  return sound;
}

__regargs void DeleteSound(SoundT *sound) {
  if (sound) {
    MemFree(sound->sample);
    MemFree(sound);
  }
}

void AudioSetVolume(ChanT num, UBYTE level) {
  custom->aud[num].ac_vol = (UWORD)level;
}

void AudioSetPeriod(ChanT num, UWORD period) {
  custom->aud[num].ac_per = period;
}

void AudioSetSampleRate(ChanT num, UWORD rate) {
  custom->aud[num].ac_per = (UWORD)(3579546L / rate);
}

void AudioAttachSample(ChanT num, APTR data, ULONG length) {
  custom->aud[num].ac_ptr = (UWORD *)data;
  custom->aud[num].ac_len = (UWORD)(length >> 1);
}

void AudioAttachSound(ChanT num, SoundT *sound) {
  AudioAttachSample(num, sound->sample, sound->length);
  AudioSetSampleRate(num, sound->rate);
}

void AudioPlay(ChanT num) {
  custom->dmacon = DMAF_SETCLR | (1 << (DMAB_AUD0 + num));
}

void AudioStop(ChanT num) {
  custom->dmacon = (1 << (DMAB_AUD0 + num));
}
