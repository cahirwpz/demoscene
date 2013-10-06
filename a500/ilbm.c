#include <exec/memory.h>
#include <proto/exec.h>

#include "gfx.h"
#include "iff.h"

#define ID_ILBM MAKE_ID('I', 'L', 'B', 'M')
#define ID_BMHD MAKE_ID('B', 'M', 'H', 'D')
#define ID_CMAP MAKE_ID('C', 'M', 'A', 'P')

/* Masking technique. */
#define mskNone                0
#define mskHasMask             1
#define mskHasTransparentColor 2
#define mskLasso               3

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

__regargs static void UnRLE(BYTE *data, LONG size, BYTE *uncompressed) {
  BYTE *src = data;
  BYTE *end = data + size;
  BYTE *dst = uncompressed;

  do {
    WORD code = *src++;

    if (code < 0) {
      BYTE b = *src++;
      WORD n = -code;

      do { *dst++ = b; } while (--n != -1);
    } else {
      WORD n = code;

      do { *dst++ = *src++; } while (--n != -1);
    }
  } while (src < end);
}

__regargs static void Deinterleave(BYTE *data, BitmapT *bitmap) { 
  WORD modulo = bitmap->bytesPerRow * (bitmap->depth - 1);
  WORD planeNum = bitmap->depth - 1;

  do {
    BYTE *src = data + bitmap->bytesPerRow * planeNum;
    BYTE *dst = bitmap->planes[planeNum];
    WORD rows = bitmap->height;

    do {
      WORD bytes = bitmap->bytesPerRow - 1;

      do { *dst++ = *src++; } while (--bytes != -1);

      src += modulo;
    } while (--rows);
  } while (--planeNum >= 0);
}

__regargs BitmapT *LoadILBM(const char *filename, BOOL interleaved) {
  BitmapT *bitmap = NULL;
  PaletteT *palette = NULL;
  IffFileT iff;

  if (OpenIff(&iff, filename)) {
    if (iff.header.type == ID_ILBM) {
      BOOL compression = FALSE;

      while (ParseChunk(&iff)) {
        BitmapHeaderT bmhd;

        switch (iff.chunk.type) {
          case ID_BMHD:
            ReadChunk(&iff, &bmhd);
            bitmap = NewBitmap(bmhd.w, bmhd.h, bmhd.nPlanes, interleaved);
            compression = bmhd.compression;
            break;

          case ID_CMAP:
            palette = NewPalette(iff.chunk.length / sizeof(ColorT));
            ReadChunk(&iff, palette->colors);
            break;
        
          case ID_BODY:
            {
              BYTE *data = AllocMem(iff.chunk.length, MEMF_PUBLIC);
              LONG size = iff.chunk.length;

              ReadChunk(&iff, data);

              if (compression) {
                LONG newSize = bitmap->bplSize * bitmap->depth;
                BYTE *uncompressed = AllocMem(newSize, MEMF_PUBLIC);

                UnRLE(data, size, uncompressed);
                FreeMem(data, size);

                data = uncompressed;
                size = newSize;
              }

              if (!interleaved)
                Deinterleave(data, bitmap);
              else
                CopyMem(data, bitmap->planes[0], bitmap->bplSize * bitmap->depth);

              FreeMem(data, size);
            }
            break;

          default:
            SkipChunk(&iff);
            break;
        }
      }

      if (bitmap)
        bitmap->palette = palette;
    }

    CloseIff(&iff);
  }

  return bitmap;
}
