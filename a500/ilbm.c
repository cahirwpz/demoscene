#include <exec/memory.h>
#include <proto/exec.h>

#include "gfx.h"
#include "iff.h"

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
      WORD n = 1 - code;

      do { *dst++ = b; } while (--n);
    } else {
      WORD n = code + 1;

      do { *dst++ = *src++; } while (--n);
    }
  } while (src < end);
}

__regargs static void Deinterleave(BYTE *data, BitmapT *bitmap) { 
  WORD modulo = bitmap->bytesPerRow * (bitmap->depth - 1);
  WORD i;

  for (i = 0; i < bitmap->depth; i++) {
    BYTE *src = data + bitmap->bytesPerRow * i;
    BYTE *dst = bitmap->planes[i];
    WORD rows = bitmap->height;

    do {
      WORD bytes = bitmap->bytesPerRow;

      do {
        *dst++ = *src++;
      } while (--bytes);

      src += modulo;
    } while (--rows);
  }
}

__regargs BitmapT *LoadILBM(const char *filename) {
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
            bitmap = NewBitmap(bmhd.w, bmhd.h, bmhd.nPlanes);
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

              Deinterleave(data, bitmap);
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
