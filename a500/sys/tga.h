#ifndef __TGA_H__
#define __TGA_H__

#include "pixmap.h"

__regargs PixmapT *
LoadTGA(CONST STRPTR filename, PixmapTypeT type, ULONG memoryFlags);

#endif
