#ifndef __BEAMPOS_H__
#define __BEAMPOS_H__

#include <custom.h>

typedef struct hpos {
  short hpos;
} hpos;

typedef struct vpos {
  short vpos;
} vpos;

#define HP(x) (hpos){.hpos = (x)}
#define VP(y) (vpos){.vpos = (y)}

#define _XS 0x81
#define _YS 0x2c

#define _X(x) ((x) + _XS)
#define _Y(y) ((y) + _YS)

/* Beam position defs for PAL signal. */
#define X(x) HP(_X(x))
#define Y(y) VP(_Y(y))

/* Last horizontal beam position copper can reliably wait on. */
#define LASTHP HP(0xDE << 1)

static inline void WaitLine(vpos vp) {
  uint32_t line = vp.vpos;
  while ((custom->vposr_ & 0x1ff00) != ((line << 8) & 0x1ff00));
}

static inline void WaitVBlank(void) { WaitLine(VP(303)); }

#endif /* !__BEAMPOS_H__ */
