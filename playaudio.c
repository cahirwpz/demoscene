#include <devices/ahi.h>
#include <exec/exec.h>
#include <proto/ahi.h>
#include <proto/dos.h>
#include <proto/exec.h>

#include <stdio.h>
#include <stdlib.h>

#ifndef uint8_t
typedef unsigned char uint8_t;
#endif

#ifndef uint16_t
typedef unsigned short uint16_t;
#endif

#ifndef uint32_t
typedef unsigned long uint32_t;
#endif

#define max(a, b) ((a) > (b) ? (a) : (b))

struct Library *AHIBase;

typedef struct AudioStream {
  struct MsgPort *msgPort;
  struct AHIRequest *ioReq;
  struct AHIAudioCtrl *ctrl;
  struct AHISampleInfo sample[2];
  struct Hook soundHook;

  /* double buffer stuff */
  size_t signal;
  struct Task *task;
  int buffer;

  /* stream */
  BPTR file;

  size_t bufLen;
  size_t sampleFreq;
  size_t sampleWidth;
} AudioStreamT;

typedef struct DiskSample {
  uint16_t padding : 14;
  uint16_t b16 : 1;
  uint16_t stereo : 1;
  uint16_t rate;
  uint32_t length;
} DiskSampleT;

__saveds ULONG SoundFunc(struct Hook *hook asm("a0"),
                         struct AHIAudioCtrl *actrl asm("a2"),
                         struct AHISoundMessage *smsg asm("a1"))
{
  AudioStreamT *audio = actrl->ahiac_UserData;
  audio->buffer = !audio->buffer;
  AHI_SetSound(0, audio->buffer, 0, 0, actrl, 0L);
  Signal(audio->task, 1L << audio->signal);
  return 0L;
}

void AudioStreamOpen(AudioStreamT *audio, const char *filename) {
  ULONG id;
  DiskSampleT sample;

  /* Open audio stream file and read the header. */
  audio->file = Open(filename, MODE_OLDFILE);

  Read(audio->file, &sample, sizeof(DiskSampleT));

  printf("Audio stream '%s' info: %d bit, %s, %dHz, %ld samples.\n",
         filename,
         sample.b16 ? 16 : 8,
         sample.stereo ? "stereo" : "mono",
         sample.rate,
         sample.length);

  /* Allocate a signal for swap-buffer events. */
  audio->signal = AllocSignal(-1);
  audio->buffer = 0;
  audio->task = FindTask(NULL);

  memset(&audio->soundHook, 0, sizeof(struct Hook));
  audio->soundHook.h_Entry = &SoundFunc;

  /* Open 'ahi.device' version 4 or greater. */
  audio->msgPort = CreateMsgPort();
  audio->ioReq = (struct AHIRequest *)
    CreateIORequest(audio->msgPort, sizeof(struct AHIRequest));

  audio->ioReq->ahir_Version = 4;
  OpenDevice(AHINAME, AHI_NO_UNIT, (struct IORequest *)audio->ioReq, 0L);
  AHIBase = (struct Library *)audio->ioReq->ahir_Std.io_Device;

  /* Find best Paula mode. */
  id = AHI_BestAudioID(AHIDB_MinMixFreq, 22050,
                       AHIDB_AudioID,    0x20000,
                       AHIDB_Bits,       14,
                       AHIDB_HiFi,       0,
                       AHIDB_Panning,    1,
                       AHIDB_Stereo,     sample.stereo,
                       TAG_DONE);

  /* WinUAE hack */
  id = 0x1A0000;

  /* Id should be checked here, in case paula driver is not installed. */
  printf("Chosen 0x%.8lx AHI AudioID\n", id);

  /* Allocate audio hardware. */
  audio->ctrl = AHI_AllocAudio(AHIA_AudioID,   id,
                               AHIA_MixFreq,   22050,
                               AHIA_Channels,  sample.stereo ? 2 : 1,
                               AHIA_Sounds,    2,
                               AHIA_SoundFunc, (ULONG)&audio->soundHook,
                               AHIA_UserData,  (ULONG)audio,
                               TAG_DONE);

  /* Set-up sample buffers. */
  {
    int sampleType, sampleWidth;

    switch ((sample.b16 << 1) | sample.stereo) {
      case 0:
        sampleType = AHIST_M8S;
        sampleWidth = 1;
        break;

      case 1:
        sampleType = AHIST_S8S;
        sampleWidth = 2;
        break;

      case 2:
        sampleType = AHIST_M16S;
        sampleWidth = 2;
        break;

      case 3:
        sampleType = AHIST_S16S;
        sampleWidth = 4;
        break;

      default:
        exit(0L);
        break;
     }

    audio->bufLen = sample.rate / 2;
    audio->sampleFreq = sample.rate;
    audio->sampleWidth = sampleWidth;

    audio->sample[0].ahisi_Type = sampleType;
    audio->sample[0].ahisi_Length = audio->bufLen;
    audio->sample[0].ahisi_Address =
      AllocVec(audio->bufLen * sampleWidth, MEMF_PUBLIC|MEMF_CLEAR);
    AHI_LoadSound(0, AHIST_DYNAMICSAMPLE, &audio->sample[0], audio->ctrl);

    audio->sample[1].ahisi_Type = sampleType;
    audio->sample[1].ahisi_Length = audio->bufLen;
    audio->sample[1].ahisi_Address =
      AllocVec(audio->bufLen * sampleWidth, MEMF_PUBLIC|MEMF_CLEAR);
    AHI_LoadSound(1, AHIST_DYNAMICSAMPLE, &audio->sample[1], audio->ctrl);
  }
}

size_t AudioStreamFeed(AudioStreamT *audio) {
  size_t requested = audio->bufLen * audio->sampleWidth;
  size_t obtained = Read(audio->file,
                         audio->sample[audio->buffer].ahisi_Address,
                         requested);

  if (obtained < requested)
    /* Clear rest of buffer */
    memset(audio->sample[audio->buffer].ahisi_Address, 0, requested - obtained);

  return obtained / audio->sampleWidth;
}

BOOL AudioStreamPlay(AudioStreamT *audio) {
  if (AHI_ControlAudio(audio->ctrl,
                       AHIC_Play, 1,
                       TAG_DONE) == AHIE_OK)
  {
    struct AHIEffMasterVolume masterVol;

    masterVol.ahie_Effect = AHIET_MASTERVOLUME;
    masterVol.ahiemv_Volume = 0x40000;
    AHI_SetEffect(&masterVol, audio->ctrl);

    AudioStreamFeed(audio);

    // The new AHI_PlayA() function is demonstrated...
    AHI_Play(audio->ctrl,
             AHIP_BeginChannel, 0,
             AHIP_Freq,         audio->sampleFreq,
             AHIP_Vol,          0x10000L,
             AHIP_Pan,          0x8000L,
             AHIP_Sound,        0,
             AHIP_Offset,       0,
             AHIP_Length,       0,
             AHIP_EndChannel,   0,
             TAG_DONE);

    return TRUE;
  }

  return FALSE;
}

void AudioStreamStop(AudioStreamT *audio) {
  AHI_ControlAudio(audio->ctrl,
                   AHIC_Play, 0,
                   TAG_DONE);
}

void AudioStreamClose(AudioStreamT *audio) {
  FreeVec(audio->sample[0].ahisi_Address);
  FreeVec(audio->sample[1].ahisi_Address);
  AHI_FreeAudio(audio->ctrl);
  CloseDevice((struct IORequest *)audio->ioReq);
  DeleteIORequest((struct IORequest *)audio->ioReq);
  DeleteMsgPort(audio->msgPort);
  FreeSignal(audio->signal);
  Close(audio->file);
}

int main() {
  AudioStreamT audio;

  AudioStreamOpen(&audio, "test.snd");

  if (AudioStreamPlay(&audio)) {
    size_t signals;

    do {
      Write(Output(), ".", 1);

      signals = Wait((1L << audio.signal) | SIGBREAKF_CTRL_C);
     
      if (signals & SIGBREAKF_CTRL_C)
        break;

    } while (AudioStreamFeed(&audio));

    if (signals & SIGBREAKF_CTRL_C) {
      Printf("***Break\n");
    } else {
      Wait(1L << audio.signal);
    }

    AudioStreamStop(&audio);
  }

  AudioStreamClose(&audio);
}
