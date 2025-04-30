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
typedef struct { u_short hi, lo; } rgb;

void LoadColorArrayAGA(const rgb *colors, short count, int start);

#define LoadColorsAGA(colors, start) \
  LoadColorArrayAGA((colors), nitems(colors), (start))

static inline void SetColorAGA(u_short i, rgb c) {
  custom->bplcon3 = bplcon3((i << 8) & BPLCON3_BANK(7),
                            BPLCON3_BANK(7) | BPLCON3_LOCT);
  custom->color[i & 31] = c.hi;
  custom->bplcon3 = bplcon3(((i << 8) & BPLCON3_BANK(7)) | BPLCON3_LOCT,
                            BPLCON3_BANK(7) | BPLCON3_LOCT);
  custom->color[i & 31] = c.lo;
}

#endif /* !__PALETTE_H__ */
