#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <custom.h>

#define MODE_LORES  0
#define MODE_HIRES  BPLCON0_HIRES
#define MODE_DUALPF BPLCON0_DBLPF
#define MODE_LACE   BPLCON0_LACE
#define MODE_HAM    BPLCON0_HOMOD

void SetupBitplaneFetch(u_short mode, u_short xs, u_short w);

void SetupDisplayWindow(u_short mode, u_short xs, u_short ys,
                        u_short w, u_short h);

void SetupMode(u_short mode, u_short depth);

void SetupPlayfield(u_short mode, u_short depth,
                    u_short xs, u_short ys, u_short w, u_short h);

#endif
