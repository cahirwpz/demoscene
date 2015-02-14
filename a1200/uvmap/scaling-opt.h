#ifndef __UVMAP_SCALING_OPT_H__
#define __UVMAP_SCALING_OPT_H__

#include "std/types.h"

void StepperFromMap(FP16 *map asm("a0"), FP16 *stepper asm("a2"),
                    int width asm("d0"), int height asm("d1"));
void FastStepperFromMap(FP16 *map asm("a0"), FP16 *stepper asm("a2"),
                        int width asm("d0"), int height asm("d1"));
void ExpandLine8x(int16_t *dst asm("a0"), FP16 *src asm("a1"),
                  int width asm("d2"));
void FastExpandLine8x(int16_t *dst asm("a0"), FP16 *src asm("a1"),
                      int width asm("d2"));
void Increment(FP16 *x asm("a0"), FP16 *dx asm("a1"), int width asm("d1"));

#endif
