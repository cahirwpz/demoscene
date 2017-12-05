#include "hardware.h"
#include "sound.h"

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
