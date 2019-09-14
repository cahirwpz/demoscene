#ifndef __AHX_H__
#define __AHX_H__

#include "types.h"

#define AHX_SAMPLE_LEN 320

typedef struct AhxVoiceTemp {
  char Track;
  char Transpose;
  char NextTrack;
  char NextTranspose;
  char ADSRVolume;
  char pad0[91];
  void *AudioPointer;
  short AudioPeriod;
  short AudioVolume;
  char pad1[128];
} AhxVoiceTempT; /* sizeof(AhxVoiceTempT) == 232 */

typedef struct AhxInfo {
  char ExternalTiming;
  char MainVolume;
  char Subsongs;
  char SongEnd;
  char Playing;
  char pad1[3];
  int FrameCount;
  char pad2[2];
  AhxVoiceTempT VoiceTemp[4];
  char pad3[156];
  short Row;
  short Pos;
} AhxInfoT;

struct AhxPlayer {
  AhxInfoT *Public;   // pointer to ahx's public (fast) memory block
  void *Chip;         // pointer to ahx's explicit chip memory block
  u_int PublicSize;   // size of public memory (intern use only!)
  u_int ChipSize;     // size of chip memory (intern use only!)
  void *Module;       // pointer to ahxModule after InitModule
  u_int IsCIA;        // byte flag (using ANY (intern/own) cia?)
  u_int Tempo;        // word to cia tempo (normally NOT needed to xs)
};

#define AHX_SYSTEM_FRIENDLY 0
#define AHX_KILL_SYSTEM 1

int AhxInitHardware(void (*ciaInt)(void) asm("a0"), int system asm("d0"));

#define AHX_LOAD_WAVES_FILE 0
#define AHX_EXPLICIT_WAVES_PRECALCING 1
#define AHX_FILTERS 0
#define AHX_NO_FILTERS 1

int AhxInitPlayer(int waves asm("d0"), int filters asm("d1"));

int AhxInitModule(void *module asm("a0"));
int AhxInitSubSong(int subsong asm("d0"), int waitPlay asm("d1"));
void AhxInterrupt(void);
void AhxStopSong(void);
void AhxKillPlayer(void);
void AhxKillHardware(void);
void AhxNextPattern(void);
void AhxPrevPattern(void);

extern volatile struct AhxPlayer Ahx;

#endif
