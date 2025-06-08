#ifndef __BEAMPOS_H__
#define __BEAMPOS_H__

#include <custom.h>

/* Horizontal beam position in low resolution pixels (0...453) */
typedef struct hpos {
  short hpos;
} hpos;

#define HP(x) (hpos){.hpos = (x)}

/* Vertical beam position in low resolution pixels (0..311) */
typedef struct vpos {
  short vpos;
} vpos;

#define VP(y) (vpos){.vpos = (y)}

/* Default display window upper-left corner position for PAL signal. */
#define DIWHP 0x81
#define DIWVP 0x2c

typedef struct DispWin {
  hpos left;
  vpos top;
  hpos right;
  vpos bottom;
} DispWinT;

/* Beam position relative to display window upper-left corner. */
#define X(x) HP((x) + DIWHP)
#define Y(y) VP((y) + DIWVP)

/* Last horizontal beam position copper can reliably wait on. */
#define LASTHP HP(0xDE << 1)

static inline void WaitLine(vpos vp) {
  uint32_t line = vp.vpos;
  while ((custom->vposr_ & 0x1ff00) != ((line << 8) & 0x1ff00));
}

static inline void WaitVBlank(void) { WaitLine(VP(303)); }

#endif /* !__BEAMPOS_H__ */
