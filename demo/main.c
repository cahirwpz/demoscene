#include <custom.h>
#include <effect.h>
#include <ptplayer.h>
#include <color.h>
#include <copper.h>
#include <palette.h>
#include <sync.h>
#include <sprite.h>
#include <uae.h>
#include <system/task.h>
#include <system/interrupt.h>

#include "demo.h"

#define _SYSTEM
#include <system/boot.h>
#include <system/memory.h>
#include <system/cia.h>
#include <system/floppy.h>
#include <system/filesys.h>
#include <system/memfile.h>
#include <system/file.h>

extern u_char Module[];
extern u_char Samples[];
#if DELTA == 1
extern u_char SamplesSize[];
#endif

extern EffectT EmptyEffect;

short frameFromStart;
short frameTillEnd;

#include "data/demo.c"

static EffectT *AllEffects[] = {
  &EmptyEffect,
  NULL,
};

static void ShowMemStats(void) {
  Log("[Memory] CHIP: %d FAST: %d\n", MemAvail(MEMF_CHIP), MemAvail(MEMF_FAST));
}

static void LoadEffects(EffectT **effects) {
  EffectT *effect;
  for (effect = *effects; effect; effect = *effects++) { 
    EffectLoad(effect);
  }
}

static void UnLoadEffects(EffectT **effects) {
  EffectT *effect;
  for (effect = *effects; effect; effect = *effects++) { 
    EffectUnLoad(effect);
  }
}

short UpdateFrameCount(void) {
  short t = ReadFrameCounter();
  frameCount = t;
  frameFromStart = t - CurrKeyFrame(&EffectNumber);
  frameTillEnd = NextKeyFrame(&EffectNumber) - t;
  return t;
}

static volatile EffectFuncT VBlankHandler = NULL;

static int VBlankISR(void) {
  if (VBlankHandler)
    VBlankHandler();
  return 0;
}

INTSERVER(VBlankInterrupt, 0, (IntFuncT)VBlankISR, NULL);

#define SYNCPOS(pos) (((((pos) & 0xff00) >> 2) | ((pos) & 0x3f)) * 3)

static void RunEffects(void) {
  /* Set the beginning of intro. Useful for effect synchronization! */
  short pos = 0;

  frameCount = SYNCPOS(pos);
  SetFrameCounter(frameCount);
  PtData.mt_SongPos = pos >> 8;
  PtData.mt_PatternPos = (pos & 0x3f) << 4;
  PtEnable = -1;

  AddIntServer(INTB_VERTB, VBlankInterrupt);

  for (;;) {
    static short prev = -1;
    short curr = TrackValueGet(&EffectNumber, frameCount);

    // Log("prev: %d, curr: %d, frameCount: %d\n", prev, curr, frameCount);

    if (prev != curr) {
      if (prev >= 0) {
        VBlankHandler = NULL;
        EffectKill(AllEffects[prev]);
        ShowMemStats();
      }
      if (curr == -1)
        break;
      EffectInit(AllEffects[curr]);
      VBlankHandler = AllEffects[curr]->VBlank;
      ShowMemStats();
      Log("[Effect] Transition to %s took %d frames!\n",
          AllEffects[curr]->name, ReadFrameCounter() - lastFrameCount);
      lastFrameCount = ReadFrameCounter() - 1;
    }

    {
      EffectT *effect = AllEffects[curr];
      short t = UpdateFrameCount();
      if ((lastFrameCount != frameCount) && effect->Render)
        effect->Render();
      lastFrameCount = t;
    }

    prev = curr;
  }

  RemIntServer(INTB_VERTB, VBlankInterrupt);
}

#if DELTA == 1
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

extern void LoadDemo(void);

int main(void) {
  /* NOP that triggers fs-uae debugger to stop and inform GDB that it should
   * fetch segments locations to relocate symbol information read from file. */
  asm volatile("exg %d7,%d7");

  ResetSprites();
  LoadDemo();
#if DELTA == 1
  Log("[Init] Decoding samples\n");
  DecodeSamples(Samples, (int)SamplesSize);
#endif
  PtInstallCIA();
  PtInit(Module, Samples, 0);

  LoadEffects(AllEffects);
  UaeWarpMode(0);
  RunEffects();

  PtEnd();
  PtRemoveCIA();

  UnLoadEffects(AllEffects);

  KillFileSys();

  return 0;
}
