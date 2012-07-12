#include "txtgen/txtgen.h"

void ChannelClear(ChannelT *D, uint8_t value) {
  size_t size = GetChannelSize(D);
  size_t i;

  for (i = 0; i < size; i++)
    SetSample(D, i, value);
}

void ChannelAdd(ChannelT *D, ChannelT *A, ChannelT *B) {
  size_t size = GetChannelSize(D);
  size_t i;

  for (i = 0; i < size; i++) {
    size_t value = GetSample(A, i) + GetSample(B, i);

    if (value > 255)
      value = 255;

    SetSample(D, i, value);
  }
}

void ChannelMul(ChannelT *D, ChannelT *A, ChannelT *B) {
  size_t size = GetChannelSize(D);
  size_t i;

  for (i = 0; i < size; i++) {
    size_t value = (GetSample(A, i) * GetSample(B, i)) >> 8;

    SetSample(D, i, value);
  }
}

void ChannelMix(ChannelT *D, ChannelT *A, ChannelT *B, size_t percent) {
  size_t size = GetChannelSize(D);
  size_t i;

  for (i = 0; i < size; i++) {
    size_t value = ((GetSample(A, i) * percent) >> 8) +
                   ((GetSample(B, i) * (256 - percent)) >> 8);

    SetSample(D, i, value);
  }
}

void ChannelCopy(ChannelT *D, ChannelT *A) {
  size_t size = GetChannelSize(D);
  size_t i;

  for (i = 0; i < size; i++)
    SetSample(D, i, GetSample(A, i));
}

void ChannelSwap(ChannelT *D, ChannelT *A) {
  size_t size = GetChannelSize(D);
  size_t i;

  for (i = 0; i < size; i++) {
    size_t d = GetSample(D, i);
    size_t a = GetSample(A, i);

    SetSample(A, i, d);
    SetSample(D, i, a);
  }
}

void ChannelMax(ChannelT *D, ChannelT *A) {
  size_t size = GetChannelSize(D);
  size_t i;

  for (i = 0; i < size; i++) {
    size_t value = max(GetSample(D, i), GetSample(A, i));

    SetSample(D, i, value);
  }
}

void ChannelShade(ChannelT *D, ChannelT *A, ChannelT *B) {
  size_t size = GetChannelSize(D);
  size_t i;

  for (i = 0; i < size; i++) {
    size_t shade = GetSample(A, i) * 2;
    size_t value = GetSample(B, i);

    if (shade < 256) {
      value = (value * shade) >> 8;
    } else {
      value += (((255 - value) * (shade - 256)) >> 8);
    }

    SetSample(D, i, value);
	}
}

void ChannelMixWithMap(ChannelT *D, ChannelT *A, ChannelT *B, ChannelT *C) {
  size_t size = GetChannelSize(D);
  size_t i;

  for (i = 0; i < size; i++) {
    size_t percent = GetSample(C, i);
    size_t value = ((GetSample(A, i) * percent) >> 8) +
                   ((GetSample(B, i) * (255 - percent)) >> 8);

    SetSample(D, i, value);
	}
}
