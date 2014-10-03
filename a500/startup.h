#ifndef __STARTUP_H__
#define __STARTUP_H__

#include <exec/types.h>

#ifndef X
#define X(x) ((x) + 0x81)
#endif

#ifndef Y
#define Y(y) ((y) + 0x2c)
#endif

extern LONG frameCount;
extern LONG lastFrameCount;

typedef struct Effect {
  /* AmigaOS is active during this step. Loads resources from disk. */
  void (*Load)();
  /* Frees all resources allocated by "Load" step. */
  void (*UnLoad)();
  /*
   * Does all initialization steps required to launch the effect.
   * 1) Allocate required memory.
   * 2) Run all precalc routines.
   * 2) Generate copper lists.
   * 3) Set up interrupts and DMA channels.
   */
  void (*Init)();
  /* Frees all resources allocated by "Init" step. */
  void (*Kill)();
  /* Renders single frame of an effect. */
  void (*Render)();
  /* Handles interrupts originating from vertical blank, blitter and copper. */
  void (*InterruptHandler)();
  /* Handles all events and returns FALSE to break the loop. */
  BOOL (*HandleEvent)();
} EffectT;

#endif
