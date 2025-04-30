#ifndef __PLAYFIELD_H__
#define __PLAYFIELD_H__

#include <beampos.h>

/* for OCS/ECS */
#define MODE_LORES 0
#define MODE_HIRES __BIT(15)     /* BPLCON0_HIRES */
#define MODE_DUALPF __BIT(10)    /* BPLCON0_DBLPF */
#define MODE_LACE __BIT(2)       /* BPLCON0_LACE */
#define MODE_HAM __BIT(11)       /* BPLCON0_HOMOD */
/* for AGA */
#define MODE_SHRES __BIT(6)      /* BPLCON0_SHRES */
#define MODE_FMODE(x) ((x) & 3)  /* FMODE_BPAGEM, FMODE_BLP32 */
#define MODE_LINEDBL __BIT(14)   /* FMODE_BSCAN2 */

/* Configures ddfstrt/ddfstop/bplcon1/fmode registers */
void SetupBitplaneFetch(u_short mode, hpos xstart, u_short width);

/* Configures diwstrt/diwstop registers
 * Arguments must be always specified in low resolution coordinates. */
void SetupDisplayWindow(u_short mode, hpos xstart, vpos ystart,
                        u_short width, u_short height);

/* Configures bplcon0/bplcon2/bplcon3 registers */
void SetupMode(u_short mode, u_short depth);

void SetupPlayfield(u_short mode, u_short depth,
                    hpos xstart, vpos ystart, u_short width, u_short height);

#endif
