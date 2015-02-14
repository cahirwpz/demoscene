#include <devices/audio.h>
#include <hardware/intbits.h>
#include <exec/exec.h>
#include <proto/exec.h>

#include "std/debug.h"
#include "std/memory.h"
#include "system/audio.h"
#include "system/hardware.h"

typedef struct MsgPort MsgPortT;
typedef struct IOAudio IOAudioT;
typedef struct IORequest IORequestT;
typedef struct Interrupt InterruptT;

typedef struct AudioInt {
  InterruptT server;
  InterruptT *oldServer;
  void (*handler)(ChanT num, PtrT userData);
  PtrT userData;
} AudioIntT;

typedef struct Audio {
  /* structures used by audio.device */
  MsgPortT *msgPort;
  IORequestT *ioReq;
  AudioIntT audioInt[4];
} AudioT;

static AudioT TheAudio;
static bool AudioInitialized = false;

__saveds __interrupt static int AudioServer(ChanT num asm("a1")) {
  AudioIntT *audioInt = &TheAudio.audioInt[num];

  if (audioInt->handler)
    audioInt->handler(num, audioInt->userData);

  custom->intreq = 1 << (INTB_AUD0 + num);

  return 0;
}

bool InitAudio() {
  AudioT *audio = &TheAudio;

  ASSERT(!AudioInitialized, "Audio has been already initialized.");

  memset(audio, 0, sizeof(TheAudio));

  if ((audio->msgPort = CreateMsgPort())) {
    audio->ioReq = (IORequestT *)NewRecord(IOAudioT);

    {
      IOAudioT *ioReq = (IOAudioT *)audio->ioReq;
      uint8_t channels[] = { 15 };

      ioReq->ioa_Request.io_Message.mn_ReplyPort = audio->msgPort;
      ioReq->ioa_Request.io_Message.mn_Node.ln_Pri = ADALLOC_MAXPREC;
      ioReq->ioa_Request.io_Command = ADCMD_ALLOCATE;
      ioReq->ioa_Request.io_Flags = ADIOF_NOWAIT;
      ioReq->ioa_AllocKey = 0;
      ioReq->ioa_Data = channels;
      ioReq->ioa_Length = sizeof(channels);
    }

    if (OpenDevice(AUDIONAME, 0L, audio->ioReq, 0L) == 0) {
      int i;

      for (i = 0; i < 4; i++) {
        InterruptT *audioInt = &audio->audioInt[i].server;

        audioInt->is_Node.ln_Type = NT_INTERRUPT;
        audioInt->is_Node.ln_Pri = 127;
        audioInt->is_Node.ln_Name = "AudioServer";
        audioInt->is_Data = (APTR)i;
        audioInt->is_Code = (APTR)AudioServer;

        audio->audioInt[i].oldServer = SetIntVector(INTB_AUD0 + i, audioInt);
      }

      AudioInitialized = true;
      return true;
    } else {
      LOG("OpenDevice(%s, ...) failed.", AUDIONAME);
    }

    MemUnref(audio->ioReq);
    DeleteMsgPort(audio->msgPort);
  } else {
    LOG("CreateMsgPort failed.");
  }

  return false;
}

void KillAudio() {
  AudioT *audio = &TheAudio;
  int i;

  ASSERT(AudioInitialized, "Audio has not been initialized yet.");

  for (i = 0; i < 4; i++)
    SetIntVector(INTB_AUD0 + i, audio->audioInt[i].oldServer);

  CloseDevice(audio->ioReq);
  MemUnref(audio->ioReq);
  DeleteMsgPort(audio->msgPort);
}

bool AudioFilter(bool on) {
  bool old = !(ciaa->ciapra & CIAF_LED);

  if (on) {
    ciaa->ciapra &= ~CIAF_LED;
  } else {
    ciaa->ciapra |= CIAF_LED;
  }

  return old;
}

void AudioSetVolume(ChanT num, uint8_t level) {
  custom->aud[num].ac_vol = (uint16_t)level;
}

void AudioSetPeriod(ChanT num, uint16_t period) {
  custom->aud[num].ac_per = period;
}

void AudioSetSampleRate(ChanT num, uint16_t rate) {
  custom->aud[num].ac_per = (uint16_t)(3579546L / rate);
}

void AudioAttachSamples(ChanT num, uint16_t *data, uint32_t length) {
  custom->aud[num].ac_ptr = data;
  custom->aud[num].ac_len = (uint16_t)(length >> 1);
}

void AudioPlay(ChanT num) {
  custom->dmacon = DMAF_SETCLR | (1 << (DMAB_AUD0 + num));
}

void AudioStop(ChanT num) {
  custom->dmacon = (1 << (DMAB_AUD0 + num));
}

void AudioIntActivate(ChanT num, bool on) {
  custom->intena = (on ? INTF_SETCLR : 0) | (1 << (INTB_AUD0 + num));
}

void AudioIntSetHandler(ChanT num, AudioIntHandlerT handler, PtrT userData) {
  AudioIntT *audioInt = &TheAudio.audioInt[num];

  audioInt->handler = handler;
  audioInt->userData = userData;
}

uint16_t *AllocAudioData(size_t length) {
  return AllocVec((length + 1) % ~1, MEMF_CHIP | MEMF_CLEAR);
}

void FreeAudioData(uint16_t *data) {
  FreeVec(data);
}
