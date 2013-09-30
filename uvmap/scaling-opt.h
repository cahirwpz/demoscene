#ifndef __UVMAP_SCALING_OPT_H__
#define __UVMAP_SCALING_OPT_H__

#include "std/types.h"

void StepperFromMap(Q16T *map asm("a0"), Q16T *stepper asm("a2"),
                    int width asm("d0"), int height asm("d1"));
void FastStepperFromMap(Q16T *map asm("a0"), Q16T *stepper asm("a2"),
                        int width asm("d0"), int height asm("d1"));
void ExpandLine8x(int16_t *dst asm("a0"), Q16T *src asm("a1"),
                  int width asm("d2"));
void FastExpandLine8x(int16_t *dst asm("a0"), Q16T *src asm("a1"),
                      int width asm("d2"));
void Increment(Q16T *x asm("a0"), Q16T *dx asm("a1"), int width asm("d1"));

#endif
