#ifndef __IFF_H__
#define __IFF_H__

#include <exec/types.h>

#define ID_FORM 0x464F524D
#define ID_BMHD 0x424D4844
#define ID_CMAP 0x434D4150
#define ID_BODY 0x424F4459

typedef struct IffHeader {
  LONG magic;
  LONG length;
  LONG type;
} IffHeaderT;

typedef struct IffChunk {
  LONG type;
  LONG length;
} IffChunkT;

/* Masking technique. */
#define mskNone                0
#define mskHasMask             1
#define mskHasTransparentColor 2
#define mskLasso               3

/* Compression algorithm applied to the rows of all source and mask planes. */
#define cmpNone        0
#define cmpByteRun1    1

typedef struct BitmapHeader {
  UWORD w, h;                  /* raster width & height in pixels      */
  WORD  x, y;                  /* pixel position for this image        */
  UBYTE nPlanes;               /* # source bitplanes                   */
  UBYTE masking;
  UBYTE compression;
  UBYTE pad1;                  /* unused; ignore on read, write as 0   */
  UWORD transparentColor;      /* transparent "color number" (sort of) */
  UBYTE xAspect, yAspect;      /* pixel aspect, a ratio width : height */
  WORD  pageWidth, pageHeight; /* source "page" size in pixels    */
} BitmapHeaderT;

typedef struct ColorRegister {
  UBYTE red, green, blue;      /* color intensities 0..255 */
} ColorRegisterT;

#endif
