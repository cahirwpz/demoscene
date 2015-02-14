#ifndef __STD_FP16_H__
#define __STD_FP16_H__

#include "std/types.h"
#include "std/math.h"

typedef struct fp16 {
  int16_t integer;
  uint16_t fraction;
} fp16_t;

typedef union {
  fp16_t fp;
  int32_t v;
} FP16;

#define FP16_i(a) (a).fp.integer
#define FP16_f(a) (a).fp.fraction

static inline FP16 FP16_add(FP16 a, FP16 b) {
  return (FP16)(int)(a.v + b.v);
}

static inline FP16 FP16_sub(FP16 a, FP16 b) {
  return (FP16)(int)(a.v - b.v);
}

static inline FP16 FP16_mul(FP16 a, FP16 b) {
  return (FP16)(int)(((int64_t)a.v * (int64_t)b.v) / 65536);
}

static inline FP16 FP16_div(FP16 a, FP16 b) {
  return (FP16)(int)(((int64_t)a.v << 16) / b.v);
}

static inline FP16 FP16_float(float value) {
  return (FP16)(int)lroundf(value * 65536.0f);
}

static inline FP16 FP16_int(int value) {
  return (FP16)(int)(value << 16);
}

static inline int FP16_rint(FP16 value) {
  if (value.v < 0)
    value.v -= 0x7fff;
  else
    value.v += 0x8000;

  return FP16_i(value);
}

static inline int FP16_rintf(FP16 value) {
  value.v += 0x8000;

  return FP16_i(value);
}

static inline FP16 FP16_rerr(FP16 value) {
  FP16 orig = value;

  if (value.v < 0)
    value.v -= 0x7fff;
  else
    value.v += 0x8000;

  orig.v -= value.v & 0xffff0000;

  return orig;
}

static inline FP16 FP16_recip(int value) {
  FP16 result = (FP16)(int)0;

  if (value > 1)
    FP16_f(result) = 0x10000 / value;
  else if (value == 1)
    FP16_i(result) = 1;

  /* the function doesn't handle reciprocal of zero */

  return result;
}

__regargs FP16 *CalcSineTableFP16(size_t n, size_t frequency, float amplitude,
                                  float shift);

#endif
