#ifndef __GFX_PIXELS_OPT_H__
#define __GFX_PIXELS_OPT_H__

#include "std/types.h"

void RawBlitNormal(uint8_t *dst asm("a0"),
                   uint8_t *src asm("a1"),
                   const int width asm("d0"),
                   const int height asm("d1"),
                   const int sstride asm("d2"),
                   const int dstride asm("d3"));

void RawBlitTransparent(uint8_t *dst asm("a0"),
                        uint8_t *src asm("a1"),
                        const int width asm("d0"),
                        const int height asm("d1"),
                        const int sstride asm("d2"), 
                        const int dstride asm("d3"));

void RawBlitColorMap(uint8_t *dst asm("a0"),
                     uint8_t *src asm("a1"),
                     const int width asm("d0"),
                     const int height asm("d1"),
                     const int sstride asm("d2"),
                     const int dstride asm("d3"),
                     uint8_t *cmap asm("a2"));

void RawBlitColorFunc(uint8_t *dst asm("a0"),
                      uint8_t *src asm("a1"),
                      const int width asm("d0"),
                      const int height asm("d1"),
                      const int sstride asm("d2"),
                      const int dstride asm("d3"),
                      uint8_t *cfunc asm("a2"));


void RawAddAndClamp(uint8_t *dst asm("a0"), uint8_t *src asm("a1"),
                    int size asm("d0"), uint8_t value asm("d1"));

void RawSubAndClamp(uint8_t *dst asm("a0"), uint8_t *src asm("a1"),
                    int size asm("d0"), uint8_t value asm("d1"));

#endif
