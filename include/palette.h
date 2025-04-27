#ifndef __PALETTE_H__
#define __PALETTE_H__

#include "common.h"
#include "custom.h"

/* Palette handling for OCS/ECS with RGB12 colors. */
void LoadColorArray(const u_short *colors, short count, int start);

#define LoadColors(colors, start) \
  LoadColorArray((colors), nitems(colors), (start))

static inline void SetColor(u_short i, u_short rgb) {
  custom->color[i] = rgb;
}

/* Palette handling for AGA with RGB24 colors. */
typedef struct rgb {
  u_char r, g, b;
} __packed rgb;

void LoadColorArrayAGA(const rgb *colors, short count, int start);

#define LoadColorsAGA(colors, start) \
  LoadColorArrayAGA((colors), nitems(colors), (start))

static inline void SetColorAGA(u_short i, rgb c) {
  u_short hi = ((c.r & 0xf0) << 4) | (c.g & 0xf0) | ((c.b & 0xf0) >> 4);
  u_short lo = ((c.r & 0x0f) << 8) | ((c.g & 0x0f) << 4) | (c.b & 0x0f);
  custom->bplcon3 = ((i << 8) & 0xe000);
  custom->color[i & 31] = hi;
  custom->bplcon3 = ((i << 8) & 0xe000) | BPLCON3_LOCT;
  custom->color[i & 31] = lo;
}

#endif /* !__PALETTE_H__ */
