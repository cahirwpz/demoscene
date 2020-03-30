#include "hardware.h"
#include "sound.h"

void AudioSetVolume(ChanT num, u_char level) {
  custom->aud[num].ac_vol = (u_short)level;
}

void AudioSetPeriod(ChanT num, u_short period) {
  custom->aud[num].ac_per = period;
}

void AudioSetSampleRate(ChanT num, u_short rate) {
  custom->aud[num].ac_per = (u_short)(3579546L / rate);
}

void AudioAttachSample(ChanT num, void *data, u_int length) {
  custom->aud[num].ac_ptr = (u_short *)data;
  custom->aud[num].ac_len = (u_short)(length >> 1);
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
