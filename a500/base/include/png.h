#ifndef __PNG_H__
#define __PNG_H__

#include "pixmap.h"

typedef struct PngChunk PngT;

#define PNG_SKIP_IDAT 0x0001
#define PNG_SKIP_PLTE 0x0002
#define PNG_SKIP_tRNS 0x0004

__regargs PixmapT *PixmapFromPNG(PngT *png, ULONG memFlags);
__regargs PaletteT *PaletteFromPNG(PngT *png);
__regargs PngT *ReadPNG(CONST STRPTR filename, ULONG pngFlags);
__regargs void DeletePNG(PngT *png);
__regargs void PrintPNG(PngT *png);

__regargs PixmapT *
LoadPNG(CONST STRPTR filename, PixmapTypeT type, ULONG memoryFlags);

#endif
