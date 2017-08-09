#ifndef __AHX_H__
#define __AHX_H__

#include <exec/types.h>

#define AHX_SAMPLE_LEN 320

typedef struct AhxVoiceTemp {
  BYTE Track;
  BYTE Transpose;
  BYTE NextTrack;
  BYTE NextTranspose;
  BYTE ADSRVolume;
  BYTE pad0[91];
  APTR AudioPointer;
  WORD AudioPeriod;
  WORD AudioVolume;
  BYTE pad1[128];
} AhxVoiceTempT; /* sizeof(AhxVoiceTempT) == 232 */

typedef struct AhxInfo {
  BYTE ExternalTiming;
  BYTE MainVolume;
  BYTE Subsongs;
  BYTE SongEnd;
  BYTE Playing;
  BYTE pad1[3];
  LONG FrameCount;
  BYTE pad2[2];
  AhxVoiceTempT VoiceTemp[4];
  BYTE pad3[156];
  WORD Row;
  WORD Pos;
} AhxInfoT;

struct AhxPlayer {
  AhxInfoT *Public;   // pointer to ahx's public (fast) memory block
  APTR  Chip;         // pointer to ahx's explicit chip memory block
  ULONG PublicSize;   // size of public memory (intern use only!)
  ULONG ChipSize;     // size of chip memory (intern use only!)
  APTR  Module;       // pointer to ahxModule after InitModule
  ULONG IsCIA;        // byte flag (using ANY (intern/own) cia?)
  ULONG Tempo;        // word to cia tempo (normally NOT needed to xs)
};

#define AHX_SYSTEM_FRIENDLY 0
#define AHX_KILL_SYSTEM 1

LONG AhxInitHardware(VOID (*ciaInt)() asm("a0"), LONG system asm("d0"));

#define AHX_LOAD_WAVES_FILE 0
#define AHX_EXPLICIT_WAVES_PRECALCING 1
#define AHX_FILTERS 0
#define AHX_NO_FILTERS 1

LONG AhxInitPlayer(LONG waves asm("d0"), LONG filters asm("d1"));

LONG AhxInitModule(APTR module asm("a0"));
LONG AhxInitSubSong(LONG subsong asm("d0"), LONG waitPlay asm("d1"));
VOID AhxInterrupt();
VOID AhxStopSong();
VOID AhxKillPlayer();
VOID AhxKillHardware();
VOID AhxNextPattern();
VOID AhxPrevPattern();

extern struct AhxPlayer Ahx;

#endif
