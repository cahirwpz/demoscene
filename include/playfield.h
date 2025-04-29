#ifndef __PLAYFIELD_H__
#define __PLAYFIELD_H__

#include <beampos.h>

#define MODE_LORES  0
#define MODE_HIRES  BPLCON0_HIRES
#define MODE_SHRES  BPLCON0_SHRES
#define MODE_DUALPF BPLCON0_DBLPF
#define MODE_LACE   BPLCON0_LACE
#define MODE_HAM    BPLCON0_HOMOD

void SetupBitplaneFetch(u_short mode, hpos xstart, u_short width);

/* Arguments must be always specified in low resolution coordinates. */
void SetupDisplayWindow(u_short mode, hpos xstart, vpos ystart,
                        u_short width, u_short height);

void SetupMode(u_short mode, u_short depth);

void SetupPlayfield(u_short mode, u_short depth,
                    hpos xstart, vpos ystart, u_short width, u_short height);

#endif
