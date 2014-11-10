#include "iff.h"
#include "ilbm.h"
#include "memory.h"
#include "lzo.h"

#define USE_LZO 1

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

__regargs BOOL BitmapUnpack(BitmapT **bitmap) {
  BitmapT *packed = *bitmap;

  if (packed->compression) {
    BitmapT *unpacked = 
      NewBitmapCustom(packed->width, packed->height, packed->depth,
                      (packed->flags & ~BM_INTERLEAVED) | BM_DISPLAYABLE);

    ULONG packed_size = (LONG)packed->planes[1];
    BYTE *packed_data = packed->planes[0];
    ULONG unpacked_size = unpacked->bplSize * unpacked->depth;
    BYTE *unpacked_data = MemAlloc(unpacked_size, MEMF_PUBLIC);

    if (packed->compression == COMP_RLE)
      UnRLE(packed_data, packed_size, unpacked_data);
#if USE_LZO
    if (packed->compression == COMP_LZO)
      lzo1x_decompress(packed_data, packed_size,
                       unpacked_data, &unpacked_size);
#endif

    unpacked->palette = packed->palette;
    DeleteBitmap(packed);

    if (unpacked->flags & BM_INTERLEAVED)
      memcpy(unpacked->planes[0], unpacked_data,
             unpacked->bplSize * unpacked->depth);
    else
      Deinterleave(unpacked_data, unpacked);

    MemFree(unpacked_data, unpacked_size);

    *bitmap = unpacked;
    return TRUE;
  }

  return FALSE;
}

__regargs BitmapT *LoadILBMCustom(const char *filename, UWORD flags) {
  BitmapT *bitmap = NULL;
  PaletteT *palette = NULL;
  IffFileT iff;

  if (OpenIff(&iff, filename)) {
    if (iff.header.type == ID_ILBM) {
      while (ParseChunk(&iff)) {
        BitmapHeaderT bmhd;

        switch (iff.chunk.type) {
          case ID_BMHD:
            ReadChunk(&iff, &bmhd);
            if (flags & BM_KEEP_PACKED) {
              bitmap = NewBitmapCustom(bmhd.w, bmhd.h, bmhd.nPlanes,
                                       BM_MINIMAL|BM_INTERLEAVED);
              bitmap->compression = bmhd.compression;
            } else {
              bitmap = NewBitmapCustom(bmhd.w, bmhd.h, bmhd.nPlanes, flags);
            }
            break;

          case ID_CMAP:
            if (flags & BM_LOAD_PALETTE) {
              palette = NewPalette(iff.chunk.length / sizeof(ColorT));
              ReadChunk(&iff, palette->colors);
            } else {
              SkipChunk(&iff);
            }
            break;
        
          case ID_BODY:
            {
              BYTE *data = MemAlloc(iff.chunk.length, MEMF_PUBLIC);
              LONG size = iff.chunk.length;
              ReadChunk(&iff, data);

              if (flags & BM_KEEP_PACKED) {
                bitmap->planes[0] = data;
                bitmap->planes[1] = (APTR)size;
                bitmap->flags &= ~BM_MINIMAL;
              } else {
                if (bmhd.compression) {
                  ULONG newSize = bitmap->bplSize * bitmap->depth;
                  BYTE *uncompressed = MemAlloc(newSize, MEMF_PUBLIC);

                  if (bmhd.compression == COMP_RLE)
                    UnRLE(data, size, uncompressed);
#if USE_LZO
                  if (bmhd.compression == COMP_LZO)
                    lzo1x_decompress(data, size, uncompressed, &newSize);
#endif
                  MemFree(data, size);

                  data = uncompressed;
                  size = newSize;
                }

                if (flags & BM_INTERLEAVED)
                  memcpy(bitmap->planes[0], data, bitmap->bplSize * bitmap->depth);
                else
                  Deinterleave(data, bitmap);

                MemFree(data, size);
              }
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
  } else {
    Log("File '%s' missing.\n", filename);
  }

  return bitmap;
}

__regargs PaletteT *LoadPalette(const char *filename) {
  PaletteT *palette = NULL;
  IffFileT iff;

  if (OpenIff(&iff, filename)) {
    if (iff.header.type == ID_ILBM) {
      while (ParseChunk(&iff)) {
        switch (iff.chunk.type) {
          case ID_CMAP:
            palette = NewPalette(iff.chunk.length / sizeof(ColorT));
            ReadChunk(&iff, palette->colors);
            break;

          default:
            SkipChunk(&iff);
            break;
        }
      }
    }

    CloseIff(&iff);
  }

  return palette;
}
