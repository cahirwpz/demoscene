#include <devices/ahi.h>
#include <exec/exec.h>
#include <proto/ahi.h>
#include <proto/dos.h>
#include <proto/exec.h>

#include "audio/stream.h"
#include "audio/wave.h"
#include "std/debug.h"
#include "std/memory.h"
#include "system/audio.h"
#include "system/vblank.h"

struct AudioStream {
  /* hardware audio data */
  int inUse;
  AudioBufferT hwBuf[2];

  /* audio buffer data */
  bool isHungry;

  uint8_t *data;
  uint32_t dataLen;
  uint32_t position;

  /* audio stream file */
  WaveFileT wave;
};

void AudioStreamClose(AudioStreamT *audio) {
  WaveFileClose(&audio->wave);
}

TYPEDECL(AudioStreamT, (FreeFuncT)AudioStreamClose);

AudioStreamT *AudioStreamOpen(const StrT filename) {
  AudioStreamT *audio = NewInstance(AudioStreamT);

  if (WaveFileOpen(&audio->wave, filename)) {
    int i, length;

#if 0
    ASSERT(audio->wave.bitsPerSample == 8 || audio->wave.bitsPerSample == 16,
           "Only 8 or 16 bit streams supported.");
    ASSERT(audio->wave.channels == 1 || audio->wave.channels == 2,
           "Only mono or stereo streams supported.");
#endif

    ASSERT(audio->wave.bitsPerSample == 16, "Only 16 bit streams supported.");
    ASSERT(audio->wave.channels == 2, "Only stereo streams supported.");
    ASSERT(audio->wave.samplesPerSec <= 28000,
           "Maximum sampling frequency supported is 28kHz.");

    /* Set up hardware buffers. */
    audio->inUse = 0;
    length = (audio->wave.samplesPerSec + FRAMERATE - 1) / FRAMERATE;
    length = (length + 1) & ~1;

    for (i = 0; i < 2; i++) {
      audio->hwBuf[i].length = length;
      audio->hwBuf[i].left.hi = (uint8_t *)AllocAudioData(length);
      audio->hwBuf[i].left.lo = (uint8_t *)AllocAudioData(length);
      audio->hwBuf[i].right.hi = (uint8_t *)AllocAudioData(length);
      audio->hwBuf[i].right.lo = (uint8_t *)AllocAudioData(length);
    }

    /* Set ups software buffer. */
    audio->isHungry = true;
    audio->dataLen = audio->wave.samplesPerSec;
    audio->data = MemNew(audio->dataLen * SampleWidth(&audio->wave));
    audio->position = 0;

    return audio;
  }

  MemUnref(audio);

  return NULL;
}

static void AudioStreamDecode(AudioStreamT *audio, int bufNum) {
  uint8_t *data = &audio->data[audio->position * SampleWidth(&audio->wave)];
  AudioBufferT *buf = &audio->hwBuf[bufNum];

  uint8_t *leftHi = buf->left.hi;
  uint8_t *leftLo = buf->left.lo;
  uint8_t *rightHi = buf->right.hi;
  uint8_t *rightLo = buf->right.lo;
  int n = buf->length;
  bool before, after;

  before = (audio->position < audio->dataLen / 2);

  do {
    *leftLo++ = *data++ >> 2;
    *leftHi++ = *data++;
    *rightLo++ = *data++ >> 2;
    *rightHi++ = *data++;

    audio->position++;

    if (audio->position >= audio->dataLen) {
      audio->position = 0;
      data = audio->data;
    }
  } while (--n);

  after = (audio->position < audio->dataLen / 2);

  if (before != after)
    audio->isHungry = true;
}

static void AudioStreamAttach(AudioStreamT *audio, int bufNum) {
  AudioBufferT *buf = &audio->hwBuf[bufNum];

  AudioAttachSamples(CHAN_0, (uint16_t *)buf->left.hi, buf->length);
  AudioAttachSamples(CHAN_1, (uint16_t *)buf->left.lo, buf->length);
  AudioAttachSamples(CHAN_2, (uint16_t *)buf->right.hi, buf->length);
  AudioAttachSamples(CHAN_3, (uint16_t *)buf->right.lo, buf->length);
}

static void AudioPlayCallback(ChanT num, PtrT userData) {
  AudioStreamT *audio = (AudioStreamT *)userData;

  audio->inUse = (audio->inUse) ? 0 : 1;

  AudioStreamDecode(audio, audio->inUse);
  AudioStreamAttach(audio, audio->inUse);
}

void AudioStreamFeed(AudioStreamT *audio) {
  if (audio->isHungry) {
    size_t half = audio->dataLen / 2;
    uint8_t *data = audio->data;

    if (audio->position < half)
      data += half * SampleWidth(&audio->wave);

    WaveFileReadSamples(&audio->wave, data, half);

    audio->isHungry = false;
  }
}

bool AudioStreamPlay(AudioStreamT *audio) {
  int i;

  WaveFileReadSamples(&audio->wave, audio->data, audio->dataLen);
  audio->isHungry = false;

  /* set up audio parameters */
  AudioFilter(false);
  AudioSetVolume(CHAN_0, 64);
  AudioSetVolume(CHAN_1, 1);
  AudioSetVolume(CHAN_2, 64);
  AudioSetVolume(CHAN_3, 1);

  for (i = 0; i < 4; i++)
    AudioSetSampleRate(i, audio->wave.samplesPerSec);

  /* decode buffer */
  AudioStreamDecode(audio, 0);

  /* attach first one and start playing it */
  AudioStreamAttach(audio, 0);

  for (i = 0; i < 4; i++)
    AudioPlay(i);

  /* enable interrupts */
  AudioIntSetHandler(CHAN_0, AudioPlayCallback, audio);
  AudioIntActivate(CHAN_0, true);

  return true;
}

void AudioStreamStop(AudioStreamT *audio) {
  int i;

  for (i = 0; i < 4; i++)
    AudioStop(i);
}

void AudioStreamSetVolume(AudioStreamT *audio, float volume) {
}

void AudioStreamUpdatePos(AudioStreamT *audio) {
  AudioStreamStop(audio);
  WaveFileChangePosition(&audio->wave, (float)GetVBlankCounter() / FRAMERATE);
  AudioStreamPlay(audio);
}

AudioBufferT *AudioStreamGetBuffer(AudioStreamT *audio) {
  return &audio->hwBuf[audio->inUse];
}
