#ifndef __EFFECT_H__
#define __EFFECT_H__

#include <types.h>
#include <string.h>
#include <debug.h>

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

typedef enum {
  EFFECT_LOADED = 1,
  EFFECT_READY = 2,
  EFFECT_RUNNING = 4,
} EffectStateT;

typedef struct Effect {
  const char *name;
  EffectStateT state;
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

#ifdef _SYSTEM
void EffectLoad(EffectT *effect);
void EffectInit(EffectT *effect);
void EffectKill(EffectT *effect);
void EffectUnLoad(EffectT *effect);
void EffectRun(EffectT *effect);
#endif

#define EFFECT(NAME, L, U, I, K, R) \
  EffectT Effect = {                \
    .name = #NAME,                  \
    .state = 0,                     \
    .Load = (L),                    \
    .UnLoad = (U),                  \
    .Init = (I),                    \
    .Kill = (K),                    \
    .Render = (R),                  \
  };                                \

typedef struct Profile {
  const char *name;
  u_int lines, total;
  u_short min, max;
  u_short count;
} ProfileT;

#define PROFILE(NAME)                                                          \
  static ProfileT *_##NAME##_profile = &(ProfileT){                            \
    .name = #NAME, .lines = 0, .total = 0, .min = 65535, .max = 0, .count = 0};

#define ProfilerStart(NAME) _ProfilerStart(_##NAME##_profile)
#define ProfilerStop(NAME) _ProfilerStop(_##NAME##_profile)

#include <system/syscall.h>

/* Puts a task into sleep waiting for Vertical Blank interrupt.
 * Let's background task do its job. */
SYSCALL0NR(TaskWaitVBlank);

SYSCALL1NR(_ProfilerStart, ProfileT *, prof, a0);
SYSCALL1NR(_ProfilerStop, ProfileT *, prof, a0);

#endif /* !__EFFECT_H__ */
