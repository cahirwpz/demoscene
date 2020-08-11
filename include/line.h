#ifndef __LINE_H__
#define __LINE_H__

#include "gfx.h"

void CpuLineSetup(const BitmapT *bitmap, u_short plane);
void CpuLine(short x1 asm("d0"), short y1 asm("d1"), short x2 asm("d2"), short y2 asm("d3"));
void CpuEdge(short x1 asm("d0"), short y1 asm("d1"), short x2 asm("d2"), short y2 asm("d3"));

#endif
