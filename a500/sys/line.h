#ifndef __LINE_H__
#define __LINE_H__

#include "gfx.h"

__regargs void CpuLineSetup(BitmapT *bitmap, UWORD plane);
__regargs void CpuLine(WORD x0, WORD y0, WORD x1, WORD y1);
__regargs void CpuLineEdge(WORD x1, WORD y1, WORD x2, WORD y2);

#endif
