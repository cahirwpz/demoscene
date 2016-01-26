#ifndef __EFFECT_H__
#define __EFFECT_H__

#include "common.h"

#define EFFECT_LOADED 1
#define EFFECT_READY  2

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
  /* Called to prepare the data for the effect - e.g. decompress, generate,
   * move to chip memory. */
  void (*Prepare)();
  /* Keeps information about state of resources related to this effect. */
  UWORD state;
} EffectT;

#endif
