#ifndef __STARTUP_H__
#define __STARTUP_H__

#include "types.h"

/* Definitions below are useful for copper list construction. */
#ifndef X
#define X(x) ((x) + 0x81) /* horizontal beam position (display window) */
#endif

#ifndef HP
#define HP(x) (X(x) / 2) /* horizontal beam position (copper) */
#endif

#ifndef Y
#define Y(y) ((y) + 0x2c) /* vertical beam position (display window) */
#endif

#ifndef VP
#define VP(y) (Y(y) & 255) /* vertical beam position (copper) */
#endif

/*
 * Number of frames (50Hz) from time point when Render() was called first.
 */
extern int frameCount;

/*
 * The time when Render() was called previously.
 * Used to calculate how much did it take to render last frame.
 */
extern int lastFrameCount;

/*
 * When set to true effect render loop breaks.
 * Normally set by LeftMouseButton routine, but can be overwritten.
 */
extern bool exitLoop;

/* Puts a task into sleep waiting for Vertical Blank interrupt.
 * Let's background task do its job. */
void TaskWaitVBlank(void);

typedef struct Effect {
  /*
   * Executed in background task when other effect is running.
   * Precalculates data for the effect to be launched.
   */
  void (*Load)(void);
  /*
   * Frees all resources allocated by "Load" step.
   */
  void (*UnLoad)(void);
  /*
   * Does all initialization steps required to launch the effect:
   * 1) Allocate memory for buffers
   * 2) Generate copper lists
   *    (setup for display window, display data fetch, palette, sprites, etc.)
   * 3) Set up interrupts and DMA channels (copper, blitter, etc.)
   */
  void (*Init)(void);
  /*
   * Frees all resources allocated by "Init" step.
   */
  void (*Kill)(void);
  /*
   * Renders single frame of an effect.
   */
  void (*Render)(void);
} EffectT;

#endif
