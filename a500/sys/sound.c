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

void AudioIntActivate(ChanT num, BOOL on) {
  custom->intena = (on ? INTF_SETCLR : 0) | (1 << (INTB_AUD0 + num));
}

void (*AudioIntHandler)(ChanT num) = NULL;

__interrupt_handler void IntLevel4Handler() {
  if (AudioIntHandler) {
    if (custom->intreqr & INTF_AUD0)
      AudioIntHandler(CHAN_0);

    if (custom->intreqr & INTF_AUD1)
      AudioIntHandler(CHAN_1);

    if (custom->intreqr & INTF_AUD2)
      AudioIntHandler(CHAN_2);

    if (custom->intreqr & INTF_AUD3)
      AudioIntHandler(CHAN_3);
  }

  /*
   * Clear interrupt flags for this level to avoid infinite re-entering
   * interrupt handler.
   */
  custom->intreq = INTF_LEVEL4;
}
