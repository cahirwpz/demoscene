#ifndef __AHX_H__
#define __AHX_H__

#include <exec/types.h>

struct AhxPlayer {
  APTR  BSS_P;        // pointer to ahx's public (fast) memory block
  APTR  BSS_C;        // pointer to ahx's explicit chip memory block
  ULONG BSS_Psize;    // size of public memory (intern use only!)
  ULONG BSS_Csize;    // size of chip memory (intern use only!)
  APTR  Module;       // pointer to ahxModule after InitModule
  ULONG IsCIA;        // byte flag (using ANY (intern/own) cia?)
  ULONG Tempo;        // word to cia tempo (normally NOT needed to xs)
};

#define AHX_SYSTEM_FRIENDLY 0
#define AHX_KILL_SYSTEM 1

LONG AhxInitCIA(VOID (*ciaInt)() asm("a0"), LONG system asm("d0"));

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
VOID AhxKillCIA();
VOID AhxNextPattern();
VOID AhxPrevPattern();

extern struct AhxPlayer Ahx;

#endif
