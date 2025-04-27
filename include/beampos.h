#ifndef __BEAMPOS_H__
#define __BEAMPOS_H__

#include <custom.h>

typedef struct hpos {
  short hpos;
} hpos;

typedef struct vpos {
  short vpos;
} vpos;

/* Beam position defs for PAL display window setup. */
#define X(x) ((x) + 0x81)
#define Y(y) ((y) + 0x2c)

/* Beam position defs for PAL copper waiting position. */
#define HP(x) (X(x) / 2)
#define VP(y) (Y(y) & 255)

/* Last horizontal beam position copper can reliably wait on. */
#define LASTHP 0xDE

static inline void WaitLine(uint32_t line) {
  while ((custom->vposr_ & 0x1ff00) != ((line << 8) & 0x1ff00));
}

static inline void WaitVBlank(void) { WaitLine(303); }

#endif /* !__BEAMPOS_H__ */
