#ifndef __TXTGEN_H__
#define __TXTGEN_H__

#include "std/types.h"
#include "gfx/pixbuf.h"

typedef enum {
  OP_END,
  OP_GROUP,

  OP_PLASMA,
  OP_PERLIN_PLASMA,
  OP_PERLIN_NOISE,
  OP_LIGHT,
  OP_CELLS,
  OP_BITMAP,

  OP_SINE,
  OP_TWIRL,
  OP_OFFSET,
  OP_MAP,

  OP_HSV,
  OP_INVERT,
  OP_BRIGHT,
  OP_SINECOLOR,
  OP_CONTRAST,
  OP_COLORIZE,
  OP_AVERAGE,

  OP_CLEAR,
  OP_ADD,
  OP_MUL,
  OP_MIX,
  OP_COPY,
  OP_EXG,
  OP_MAX,
  OP_SHADE,
  OP_MIXMAP,

  OP_EMBOSS,
  OP_BLUR,
  OP_DIRECTIONAL_BLUR
} TxtGenOpT;

typedef enum {COMPONENT_R, COMPONENT_G, COMPONENT_B} ComponentT;
typedef struct Channel ChannelT;

size_t GetSample(ChannelT *channel asm("a0"), size_t index asm("d0"));
void SetSample(ChannelT *channel asm("a0"), size_t index asm("d0"), size_t value asm("d1"));
size_t GetChannelSize(ChannelT *channel asm("a0"));
void ChannelSetActiveComponent(ChannelT *channel, ComponentT component);

void ChannelClear(PixBufT *dst asm("a0"), uint8_t value asm("d0"));
void ChannelAdd(PixBufT *dst asm("a0"), PixBufT *src1 asm("a1"),
                PixBufT *src2 asm("a2"));
void ChannelMul(PixBufT *dst asm("a0"), PixBufT *src1 asm("a1"),
                PixBufT *src2 asm("a2"));
void ChannelMix(PixBufT *dst asm("a0"), PixBufT *src1 asm("a1"),
                PixBufT *src2 asm("a2"), size_t percent asm("d0"));
void ChannelCopy(PixBufT *dst asm("a0"), PixBufT *src asm("a1"));
void ChannelSwap(PixBufT *dst asm("a0"), PixBufT *src asm("a1"));
void ChannelMax(PixBufT *dst asm("a0"), PixBufT *src1 asm("a1"),
                PixBufT *src2 asm("a2"));
void ChannelShade(PixBufT *dst asm("a0"), PixBufT *src1 asm("a1"),
                  PixBufT *src2 asm("a2"));
void ChannelMixWithMap(PixBufT *dst asm("a0"), PixBufT *src1 asm("a1"),
                       PixBufT *src2 asm("a2"), PixBufT *map asm("a3"));

#endif
