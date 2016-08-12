#include "startup.h"
#include "hardware.h"
#include "interrupts.h"
#include "memory.h"
#include "io.h"
#include "ahx/ahx.h"

LONG __chipmem = 100 * 1024;
LONG __fastmem = 420 * 1024;
STRPTR __cwdpath = "data";

static APTR module;

static void Load() {
  module = LoadFile("jazzcat-electric_city.ahx", MEMF_PUBLIC);
  if (AhxInitPlayer(AHX_LOAD_WAVES_FILE, AHX_FILTERS) != 0)
    exit(10);
}

static void UnLoad() {
  AhxKillPlayer();
  MemFree(module);
}

static __interrupt LONG AhxPlayerIntHandler() {
  /* Handle CIA Timer A interrupt. */
  if (ciaa->ciaicr & CIAICRF_TA) {
    custom->color[0] = 0xaaa;
    AhxInterrupt();
    custom->color[0] = 0;
  }

  return 0;
}

INTERRUPT(AhxPlayerInterrupt, 10, AhxPlayerIntHandler);

static void AhxSetTempo(UWORD tempo asm("d0")) {
  ciaa->ciatalo = tempo & 0xff;
  ciaa->ciatahi = tempo >> 8;
  ciaa->ciacra |= CIACRAF_START;
}

static void Init() {
  /* Enable only CIA Timer A interrupt. */
  ciaa->ciaicr = (UBYTE)(~CIAICRF_SETCLR);
  ciaa->ciaicr = CIAICRF_SETCLR | CIAICRF_TA;
  /* Run CIA Timer A in continuous / normal mode, increment every 10 cycles. */
  ciaa->ciacra &= (UBYTE)(~CIACRAF_RUNMODE & ~CIACRAF_INMODE & ~CIACRAF_PBON);

  if (AhxInitHardware((APTR)AhxSetTempo, AHX_KILL_SYSTEM) == 0)
    if (AhxInitModule(module) == 0)
      AhxInitSubSong(0, 0);

  AddIntServer(INTB_PORTS, &AhxPlayerInterrupt);
}

static void Kill() {
  RemIntServer(INTB_PORTS, &AhxPlayerInterrupt);

  AhxStopSong();
  AhxKillHardware();
}

EffectT Effect = { Load, UnLoad, Init, Kill, NULL };
