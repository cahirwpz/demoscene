#ifndef __STARTUP_H__
#define __STARTUP_H__

#include <exec/types.h>

#ifndef X
#define X(x) ((x) + 0x80)
#endif

#ifndef Y
#define Y(y) ((y) + 0x2c)
#endif

//extern static LONG lastFrameCount;
//extern static LONG frameCount;

typedef struct Effect {
  /* Call it while OS is active. Loads resources from disk. */
  void (*Load)();
  /*
   * Does all initialization steps required to launch the effect.
   * 1) Allocate required memory.
   * 2) Run all precalc routines.
   * 2) Generate copper lists.
   * 3) Set up interrupts and DMA channels.
   */
  void (*Init)();
  /* Frees all resources allocated in "Init" and "Load" step. */
  void (*Kill)();
  /* Runs main loop of the effect. */
  void (*Loop)();
} EffectT;

#endif
