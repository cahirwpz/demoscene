#ifndef __STARTUP_H__
#define __STARTUP_H__

#include "types.h"

#ifndef X
#define X(x) ((x) + 0x81)
#endif

#ifndef HP
#define HP(x) (X(x) / 2)
#endif

#ifndef Y
#define Y(y) ((y) + 0x2c)
#endif

#ifndef VP
#define VP(y) (Y(y) & 255)
#endif

extern int frameCount;
extern int lastFrameCount;
extern struct List *VBlankEvent;

typedef struct Effect {
  /* AmigaOS is active during this step. Loads resources from disk. */
  void (*Load)(void);
  /* Frees all resources allocated by "Load" step. */
  void (*UnLoad)(void);
  /*
   * Does all initialization steps required to launch the effect.
   * 1) Allocate required memory.
   * 2) Run all precalc routines.
   * 2) Generate copper lists.
   * 3) Set up interrupts and DMA channels.
   */
  void (*Init)(void);
  /* Frees all resources allocated by "Init" step. */
  void (*Kill)(void);
  /* Renders single frame of an effect. */
  void (*Render)(void);
  /* Handles all events and returns false to break the loop. */
  bool (*HandleEvent)(void);
} EffectT;

#endif
