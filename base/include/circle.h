#ifndef __CIRCLE_H__
#define __CIRCLE_H__

#include "gfx.h"

__regargs void Circle(BitmapT *bitmap, int plane, short x0, short y0, short r);
__regargs void CircleEdge(BitmapT *bitmap, int plane, short x0, short y0, short r);

#endif
