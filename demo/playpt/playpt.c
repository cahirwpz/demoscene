#include <effect.h>
#include <custom.h>
#include <ptplayer.h>

#define _SYSTEM
#include <system/cia.h>

extern u_char Module[];
extern u_char Samples[];
#ifdef DELTA
extern u_char SamplesSize[];
#endif

#ifdef DELTA
static void DecodeSamples(u_char *smp, int size) {
  u_char data = *smp++;
  short n = (size + 7) / 8 - 1;
  short k = size & 7;

  Log("[Init] Decoding delta samples (%d bytes)\n", size);

  switch (k) {
  case 0: do { data += *smp; *smp++ = data;
  case 7:      data += *smp; *smp++ = data;
  case 6:      data += *smp; *smp++ = data;
  case 5:      data += *smp; *smp++ = data;
  case 4:      data += *smp; *smp++ = data;
  case 3:      data += *smp; *smp++ = data;
  case 2:      data += *smp; *smp++ = data;
  case 1:      data += *smp; *smp++ = data;
          } while (--n != -1);
  }
}
#endif

static void Load(void) {
#ifdef DELTA
  Log("[Init] Decoding samples\n");
  DecodeSamples(Samples, (int)SamplesSize);
#endif
}

#define SYNCPOS(pos) (((((pos) & 0xff00) >> 2) | ((pos) & 0x3f)) * 3)

static void Init(void) {
  /* Set the beginning of demo. Useful for effect synchronization! */
  short pos = 0;

  PtInstallCIA();
  PtInit(Module, Samples, 1);
  PtEnable = 1;

  frameCount = SYNCPOS(pos);
  SetFrameCounter(frameCount);
  PtData.mt_SongPos = pos >> 8;
  PtData.mt_PatternPos = (pos & 0x3f) << 4;
  PtEnable = -1;
}

static void Kill(void) {
  PtEnd();
  PtRemoveCIA();
  DisableDMA(DMAF_AUDIO);
}

EFFECT(Protracker, Load, NULL, Init, Kill, NULL, NULL);
