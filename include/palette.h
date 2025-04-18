#ifndef __PALETTE_H__
#define __PALETTE_H__

#include "common.h"
#include "custom.h"

void LoadColorArray(const u_short *colors, short count, int start);

#define LoadColors(colors, start) \
  LoadColorArray((colors), nitems(colors), (start))

static inline void SetColor(u_short i, u_short rgb) {
  custom->color[i] = rgb;
}

#endif
