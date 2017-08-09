#ifndef __CIRCLE_H__
#define __CIRCLE_H__

#include "gfx.h"

__regargs void Circle(BitmapT *bitmap, LONG plane, WORD x0, WORD y0, WORD r);
__regargs void CircleEdge(BitmapT *bitmap, LONG plane, WORD x0, WORD y0, WORD r);

#endif
