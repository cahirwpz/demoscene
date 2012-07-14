#include "txtgen/txtgen.h"

void ChannelClear(PixBufT *dst asm("a0"), uint8_t value asm("d0")) {
  memset(dst->data, value, dst->width * dst->height);
}

void ChannelAdd(PixBufT *dst asm("a0"), PixBufT *src1 asm("a1"),
                PixBufT *src2 asm("a2"))
{
  uint8_t *a = src1->data;
  uint8_t *b = src2->data;
  uint8_t *d = dst->data;
  size_t size = dst->width * dst->height;
  size_t i;

  for (i = 0; i < size; i++) {
    d[i] = min(a[i] + b[i], 255);
  }
}

void ChannelMul(PixBufT *dst asm("a0"), PixBufT *src1 asm("a1"),
                PixBufT *src2 asm("a2"))
{
  uint8_t *a = src1->data;
  uint8_t *b = src2->data;
  uint8_t *d = dst->data;
  size_t size = dst->width * dst->height;
  size_t i;

  for (i = 0; i < size; i++)
    d[i] = (a[i] * b[i]) >> 8;
}

void ChannelMix(PixBufT *dst asm("a0"), PixBufT *src1 asm("a1"),
                PixBufT *src2 asm("a2"), size_t percent asm("d0"))
{
  uint8_t *a = src1->data;
  uint8_t *b = src2->data;
  uint8_t *d = dst->data;
  size_t size = dst->width * dst->height;
  size_t i;

  for (i = 0; i < size; i++)
    d[i] = ((a[i] * percent) >> 8) + ((b[i] * (256 - percent)) >> 8);
}

void ChannelCopy(PixBufT *dst asm("a0"), PixBufT *src asm("a1")) {
  memcpy(dst->data, src->data, dst->width * dst->height);
}

void ChannelSwap(PixBufT *dst asm("a0"), PixBufT *src asm("a1")) {
  uint8_t *a = src->data;
  uint8_t *d = dst->data;
  size_t size = dst->width * dst->height;
  size_t i;
  uint8_t temp;

  for (i = 0; i < size; i++) {
    temp = d[i];
    d[i] = a[i];
    a[i] = temp;
  }
}

void ChannelMax(PixBufT *dst asm("a0"), PixBufT *src1 asm("a1"),
                PixBufT *src2 asm("a2"))
{
  uint8_t *a = src1->data;
  uint8_t *b = src2->data;
  uint8_t *d = dst->data;
  size_t size = dst->width * dst->height;
  size_t i;

  for (i = 0; i < size; i++) 
    d[i] = max(a[i], b[i]);
}

void ChannelShade(PixBufT *dst asm("a0"), PixBufT *src1 asm("a1"),
                  PixBufT *src2 asm("a2"))
{
  uint8_t *a = src1->data;
  uint8_t *b = src2->data;
  uint8_t *d = dst->data;
  size_t size = dst->width * dst->height;
  size_t i;

  for (i = 0; i < size; i++) {
    size_t shade = a[i];
    size_t value = b[i];

    if (shade < 128) {
      value = (value * shade) >> 7;
    } else {
      value += (((255 - value) * (shade - 128)) >> 7);
    }

    d[i] = value;
  }
}

void ChannelMixWithMap(PixBufT *dst asm("a0"), PixBufT *src1 asm("a1"),
                       PixBufT *src2 asm("a2"), PixBufT *map asm("a3"))
{
  uint8_t *a = src1->data;
  uint8_t *b = src2->data;
  uint8_t *m = map->data;
  uint8_t *d = dst->data;
  size_t size = dst->width * dst->height;
  size_t i;

  for (i = 0; i < size; i++)
    d[i] = ((a[i] * m[i]) >> 8) + ((b[i] * (255 - m[i])) >> 8);
}
