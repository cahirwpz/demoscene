#ifndef __LINE_H__
#define __LINE_H__

#include "gfx.h"

__regargs void CpuLineSetup(BitmapT *bitmap, UWORD plane);
void CpuLine(WORD x1 asm("d0"), WORD y1 asm("d1"), WORD x2 asm("d2"), WORD y2 asm("d3"));
void CpuEdge(WORD x1 asm("d0"), WORD y1 asm("d1"), WORD x2 asm("d2"), WORD y2 asm("d3"));

#endif
