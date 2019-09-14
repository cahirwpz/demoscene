#include <graphics/view.h>

#include "config.h"
#include "iff.h"
#include "ilbm.h"
#include "memory.h"
#if USE_LZO
#include "lzo.h"
#endif
#include "inflate.h"

#define ID_ILBM MAKE_ID('I', 'L', 'B', 'M')
#define ID_BMHD MAKE_ID('B', 'M', 'H', 'D')
#define ID_CMAP MAKE_ID('C', 'M', 'A', 'P')
#define ID_CAMG MAKE_ID('C', 'A', 'M', 'G')
#define ID_PCHG MAKE_ID('P', 'C', 'H', 'G')

/* Masking technique. */
#define mskNone                0
#define mskHasMask             1
#define mskHasTransparentColor 2
#define mskLasso               3

typedef struct BitmapHeader {
  u_short w, h;                 /* raster width & height in pixels      */
  short  x, y;                  /* pixel position for this image        */
  u_char nPlanes;               /* # source bitplanes                   */
  u_char masking;
  u_char compression;
  u_char pad1;                  /* unused; ignore on read, write as 0   */
  u_short transparentColor;     /* transparent "color number" (sort of) */
  u_char xAspect, yAspect;      /* pixel aspect, a ratio width : height */
  short  pageWidth, pageHeight; /* source "page" size in pixels    */
} BitmapHeaderT;

#define PCHG_COMP_NONE  0
#define PCHGF_12BIT     1

typedef struct PCHGHeader {
  u_short Compression;
  u_short Flags;
  short  StartLine;
  u_short LineCount;
  u_short ChangedLines;
  u_short MinReg;
  u_short MaxReg;
  u_short MaxChanges;
  u_int TotalChanges;
} PCHGHeaderT;

typedef struct LineChanges {
  u_char ChangeCount16;
  u_char ChangeCount32;
  u_short PaletteChange[0];
} LineChangesT;

__regargs static void UnRLE(char *data, int size, char *uncompressed) {
  char *src = data;
  char *end = data + size;
  char *dst = uncompressed;

  do {
    short code = *src++;

    if (code < 0) {
      char b = *src++;
      short n = -code;

      do { *dst++ = b; } while (--n != -1);
    } else {
      short n = code;

      do { *dst++ = *src++; } while (--n != -1);
    }
  } while (src < end);
}

__regargs static void Deinterleave(void *data, BitmapT *bitmap) {
  int bytesPerRow = bitmap->bytesPerRow;
  int modulo = (short)bytesPerRow * (short)(bitmap->depth - 1);
  short bplnum = bitmap->depth;
  short count = bytesPerRow / 2;
  short i = count & 7;
  short k = (count + 7) / 8;
  void **plane = bitmap->planes;

  do {
    short *src = data;
    short *dst = *plane++;
    short rows = bitmap->height;

    do {
      short n = k - 1;
      switch (i) {
        case 0: do { *dst++ = *src++;
        case 7:      *dst++ = *src++;
        case 6:      *dst++ = *src++;
        case 5:      *dst++ = *src++;
        case 4:      *dst++ = *src++;
        case 3:      *dst++ = *src++;
        case 2:      *dst++ = *src++;
        case 1:      *dst++ = *src++;
        } while (--n != -1);
      }

      src = (void *)src + modulo;
    } while (--rows);

    data += bytesPerRow;
  } while (--bplnum);
}

__regargs void BitmapUnpack(BitmapT *bitmap, u_short flags) {
  if (bitmap->compression) {
    u_int inLen = (int)bitmap->planes[1];
    void *inBuf = bitmap->planes[0];
    u_int outLen = BitmapSize(bitmap);
    void *outBuf = MemAlloc(outLen, MEMF_PUBLIC);

    if (bitmap->compression == COMP_RLE)
      UnRLE(inBuf, inLen, outBuf);
#if USE_LZO
    else if (bitmap->compression == COMP_LZO)
      lzo1x_decompress(inBuf, inLen, outBuf, &outLen);
#endif
#if USE_DEFLATE
    else if (bitmap->compression == COMP_DEFLATE)
      Inflate(inBuf, outBuf);
#endif

    MemFree(inBuf);

    bitmap->compression = COMP_NONE;
    BitmapSetPointers(bitmap, outBuf);
  }

  if ((bitmap->flags & BM_INTERLEAVED) && !(flags & BM_INTERLEAVED)) {
    u_int size = BitmapSize(bitmap);
    void *inBuf = bitmap->planes[0];
    void *outBuf = MemAlloc(size, MEMF_PUBLIC);

    bitmap->flags &= ~BM_INTERLEAVED;
    BitmapSetPointers(bitmap, outBuf);

    Deinterleave(inBuf, bitmap);
    MemFree(inBuf);
  }

  if (flags & BM_DISPLAYABLE)
    BitmapMakeDisplayable(bitmap);
}

static __regargs void LoadCAMG(IffFileT *iff, BitmapT *bitmap) {
  u_int mode;

  ReadChunk(iff, &mode);

  if (mode & HAM)
    bitmap->flags |= BM_HAM;
  if (mode & EXTRA_HALFBRITE)
    bitmap->flags |= BM_EHB;
}

static __regargs void LoadPCHG(IffFileT *iff, BitmapT *bitmap) {
  char *data = MemAlloc(iff->chunk.length, MEMF_PUBLIC);
  PCHGHeaderT *pchg = (void *)data;

  ReadChunk(iff, data);

  if ((pchg->Compression == PCHG_COMP_NONE) &&
      (pchg->Flags == PCHGF_12BIT)) 
  {
    int length = pchg->TotalChanges + bitmap->height;
    u_short *out = MemAlloc(length * sizeof(u_short), MEMF_PUBLIC);
    u_char *bitvec = (void *)pchg + sizeof(PCHGHeaderT);
    LineChangesT *line = (void *)bitvec + ((pchg->LineCount + 31) & ~31) / 8;
    short i = pchg->StartLine;

    bitmap->pchgTotal = pchg->TotalChanges;
    bitmap->pchg = out;

    while (i-- > 0)
      *out++ = 0;

    for (i = 0; i < pchg->LineCount; i++) {
      int mask = 1 << (7 - (i & 7));
      if (bitvec[i / 8] & mask) {
        u_short count = line->ChangeCount16;
        u_short *change = line->PaletteChange;

        *out++ = count;
        while (count-- > 0)
          *out++ = *change++;

        line = (void *)line + sizeof(LineChangesT) +
          (line->ChangeCount16 + line->ChangeCount32) * sizeof(u_short);
      } else {
        *out++ = 0;
      }
    }
  }

  MemFree(data);
}

static __regargs void LoadBODY(IffFileT *iff, BitmapT *bitmap, u_short flags) {
  char *data = MemAlloc(iff->chunk.length, MEMF_PUBLIC);
  int size = iff->chunk.length;

  ReadChunk(iff, data);

  if (flags & BM_KEEP_PACKED) {
    bitmap->planes[0] = data;
    bitmap->planes[1] = (void *)size;
    bitmap->flags &= ~BM_MINIMAL;
  } else {
    if (bitmap->compression) {
      u_int newSize = bitmap->bplSize * bitmap->depth;
      char *uncompressed = MemAlloc(newSize, MEMF_PUBLIC);

      if (bitmap->compression == COMP_RLE)
        UnRLE(data, size, uncompressed);
#if USE_LZO
      if (bitmap->compression == COMP_LZO)
        lzo1x_decompress(data, size, uncompressed, &newSize);
#endif
#if USE_DEFLATE
      if (bitmap->compression == COMP_DEFLATE)
        Inflate(data, uncompressed);
#endif
      MemFree(data);

      data = uncompressed;
      size = newSize;

      bitmap->compression = 0;
    }

    if (flags & BM_INTERLEAVED)
      memcpy(bitmap->planes[0], data, bitmap->bplSize * bitmap->depth);
    else
      Deinterleave(data, bitmap);

    MemFree(data);
  }
}

static __regargs BitmapT *LoadBMHD(IffFileT *iff, u_short flags) {
  BitmapHeaderT bmhd;
  BitmapT *bitmap;

  ReadChunk(iff, &bmhd);

  if (flags & BM_KEEP_PACKED)
    flags = BM_MINIMAL|BM_INTERLEAVED;

  bitmap = NewBitmapCustom(bmhd.w, bmhd.h, bmhd.nPlanes, flags);
  bitmap->compression = bmhd.compression;

  return bitmap;
}

static __regargs PaletteT *LoadCMAP(IffFileT *iff) {
  PaletteT *palette = NewPalette(iff->chunk.length / sizeof(ColorT));
  ReadChunk(iff, palette->colors);
  return palette;
}

__regargs BitmapT *LoadILBMCustom(const char *filename, u_short flags) {
  BitmapT *bitmap = NULL;
  PaletteT *palette = NULL;
  IffFileT iff;

  OpenIff(&iff, filename);

  if (iff.header.type != ID_ILBM)
    Panic("[ILBM] File '%s' has wrong type!\n", filename);

  while (ParseChunk(&iff)) {
    if (iff.chunk.type == ID_BMHD)
      bitmap = LoadBMHD(&iff, flags);
    else if (iff.chunk.type == ID_CAMG)
      LoadCAMG(&iff, bitmap);
    else if (iff.chunk.type == ID_PCHG)
      LoadPCHG(&iff, bitmap);
    else if (iff.chunk.type == ID_BODY)
      LoadBODY(&iff, bitmap, flags);
    else if ((iff.chunk.type == ID_CMAP) && (flags & BM_LOAD_PALETTE))
      palette = LoadCMAP(&iff);
    else
      SkipChunk(&iff);
  }

  if (bitmap)
    bitmap->palette = palette;

  CloseIff(&iff);

  return bitmap;
}

__regargs PaletteT *LoadPalette(const char *filename) {
  PaletteT *palette = NULL;
  IffFileT iff;

  OpenIff(&iff, filename);

  if (iff.header.type != ID_ILBM)
    Panic("[ILBM] File '%s' has wrong type!\n", filename);

  while (ParseChunk(&iff)) {
    if (iff.chunk.type == ID_CMAP) {
      palette = LoadCMAP(&iff);
      break;
    }
    SkipChunk(&iff);
  }

  CloseIff(&iff);

  return palette;
}
