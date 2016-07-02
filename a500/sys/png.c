#include <proto/exec.h>

#include "io.h"
#include "iff.h"
#include "png.h"
#include "memory.h"
#include "inflate.h"

#define PNG_ID0 MAKE_ID(0x89, 0x50, 0x4e, 0x47)
#define PNG_ID1 MAKE_ID(0x0d, 0x0a, 0x1a, 0x0a)

#define PNG_IEND MAKE_ID('I', 'E', 'N', 'D')
#define PNG_IHDR MAKE_ID('I', 'H', 'D', 'R')
#define PNG_IDAT MAKE_ID('I', 'D', 'A', 'T')
#define PNG_PLTE MAKE_ID('P', 'L', 'T', 'E')
#define PNG_tRNS MAKE_ID('t', 'R', 'N', 'S')

#define PNG_GRAYSCALE       0
#define PNG_TRUECOLOR       2
#define PNG_INDEXED         3
#define PNG_GRAYSCALE_ALPHA 4
#define PNG_TRUECOLOR_ALPHA 6

typedef struct {
  ULONG width;
  ULONG height;
  UBYTE bit_depth;
  UBYTE colour_type;
  UBYTE compression_method;
  UBYTE filter_method;
  UBYTE interlace_method;
} IHDR;

typedef struct {
  UBYTE data[0];
} IDAT;

typedef struct {
  UBYTE r, g, b;
} __attribute__((packed)) RGB;

typedef struct {
  RGB colors[0];
} PLTE;

typedef union {
  struct {
    UWORD v;
  } type0;
  struct {
    UWORD r, g, b;
  } type2;
  struct {
    ULONG length;
    UBYTE alpha[0];
  } type3;
} tRNS;

struct PngChunk {
  PngT  *next;
  ULONG id;
  ULONG size;
  UBYTE data[0];
};

static inline WORD GetRowSize(IHDR *ihdr) {
  WORD pixelWidth = ihdr->bit_depth;

  if (ihdr->colour_type == PNG_TRUECOLOR)
    pixelWidth *= 3;
  else if (ihdr->colour_type == PNG_GRAYSCALE_ALPHA)
    pixelWidth *= 2;
  else if (ihdr->colour_type == PNG_TRUECOLOR_ALPHA)
    pixelWidth *= 4;

  return (pixelWidth * ihdr->width + 7) >> 3;
}

static inline WORD PaethPredictor(WORD a, WORD b, WORD c) {
  WORD p = a + b - c;
  WORD pa = p - a;
  WORD pb = p - b;
  WORD pc = p - c;

  if (pa < 0)
    pa = -pa;
  if (pb < 0)
    pb = -pb;
  if (pc < 0)
    pc = -pc;

  if ((pa <= pb) && (pa <= pc)) return a;
  if (pb <= pc) return b;
  return c;
}

static __regargs void ReconstructImage(UBYTE *pixels, UBYTE *encoded, LONG row,
                                       WORD height)
{
  WORD i;

  for (i = 0; i < height; i++) {
    UBYTE method = *encoded++;

    if (method == 2 && i == 0)
      method = 0;

    if (method == 4 && i == 0)
      method = 1;

    /*
     * Filters are applied to bytes, not to pixels, regardless of the bit depth
     * or colour type of the image. The filters operate on the byte sequence
     * formed by a scanline.
     */

    if (method == 0) {
      CopyMem(encoded, pixels, row);
      encoded += row; pixels += row;
    } else if (method == 1) {
      UBYTE *left = pixels;
      WORD j = row - 1;
      *pixels++ = *encoded++;
      do {
        *pixels++ = *encoded++ + *left++;
      } while (--j);
    } else if (method == 2) {
      UBYTE *up = pixels - row;
      WORD j = row;
      do {
        *pixels++ = *encoded++ + *up++;
      } while (--j);
    } else if (method == 3) {
      UBYTE *left = pixels;
      WORD j = row - 1;
      if (i > 0) {
        UBYTE *up = pixels - row;
        *pixels++ = *encoded++ + *up++ / 2;
        do {
          *pixels++ = *encoded++ + (*left++ + *up++) / 2;
        } while (--j);
      } else {
        *pixels++ = *encoded++;
        do {
          *pixels++ = *encoded++ + *left++ / 2;
        } while (--j);
      }
    } else if (method == 4) {
      UBYTE *left = pixels;
      UBYTE *leftup = pixels - row;
      UBYTE *up = pixels - row;
      WORD j = row - 1;
      *pixels++ = *encoded++ + *up++;
      do {
        *pixels++ = *encoded++ + PaethPredictor(*left++, *up++, *leftup++);
      } while (--j);
    }
  }
}

/* Collapse multiple IDAT chunks into single one. */
static __regargs PngT *MergeIDAT(PngT *png) {
  PngT *idat = NULL;

  if (png && (png->id == PNG_IHDR)) {
    PngT *chk = png;
    LONG size = 0;
    LONG count = 0;

    while (chk) {
      if (chk->id == PNG_IDAT) {
        if (idat == NULL)
          idat = chk;
        size += chk->size;
        count++;
      }
      chk = chk->next;
    }

    if (count > 1) {
      Log("Number of IDAT chunks: %ld, merged size: %ld.\n", count, size);

      idat = MemAlloc(sizeof(PngT) + size, MEMF_PUBLIC);
      idat->size = size;
      idat->id = PNG_IDAT;

      /*
       * Copy IDAT chunks contents into a single chunk.
       * Free old IDAT chunks and put new one onto the list.
       */
      {
        PngT *insert = NULL, *prev = NULL;
        UBYTE *data = idat->data;

        chk = png;

        while (chk) {
          PngT *next = chk->next;
          if (chk->id == PNG_IDAT) {
            CopyMem(chk->data, data, chk->size);
            data += chk->size;
            if (insert == NULL)
              insert = prev;
            prev->next = chk->next;
            MemFree(chk);
          } else {
            prev = chk;
          }
          chk = next;
        }

        idat->next = insert->next;
        insert->next = idat;
      }
    }
  }

  return idat;
}

__regargs PixmapT *PixmapFromPNG(PngT *png, ULONG memFlags) {
  PixmapT *pixmap = NULL;

  if (png && (png->id == PNG_IHDR)) {
    IHDR *ihdr = (IHDR *)png->data;

    Log("[PNG] w: %ld, h: %ld, bd: %ld, c: %ld, i: %ld\n",
        (LONG)ihdr->width, (LONG)ihdr->height, (LONG)ihdr->bit_depth,
        (LONG)ihdr->colour_type, (LONG)ihdr->interlace_method);

    if (ihdr->interlace_method != 0) {
      Log("[PNG] Interlaced image not supported!\n");
    } else if (ihdr->bit_depth != 4 && ihdr->bit_depth != 8) {
      Log("[PNG] Images with %ld-bit components not supported!\n",
          (LONG)ihdr->bit_depth);
    } else {
      PixmapTypeT type = PM_NONE;

      if (ihdr->colour_type == PNG_GRAYSCALE)
        type = (ihdr->bit_depth == 8) ? PM_GRAY : PM_GRAY4;
      if (ihdr->colour_type == PNG_INDEXED)
        type = (ihdr->bit_depth == 8) ? PM_CMAP : PM_CMAP4;

      if (type > PM_NONE) {
        WORD row = GetRowSize(ihdr);
        LONG length = row * ihdr->height;
        PngT *idat = MergeIDAT(png);
        UBYTE *encoded = MemAlloc(length + ihdr->height, MEMF_PUBLIC);

        pixmap = NewPixmap(ihdr->width, ihdr->height, type, memFlags);

        Log("[PNG] Inflating IDAT chunk (%ld)\n", (LONG)length);
        Inflate(idat->data + 2, encoded);

        Log("[PNG] Decoding pixels.\n");
        ReconstructImage(pixmap->pixels, encoded, row, pixmap->height);

        MemFree(encoded);
      }
    }
  }

  return pixmap;
}

__regargs PaletteT *PaletteFromPNG(PngT *png) {
  PaletteT *pal = NULL;

  if (png && (png->id == PNG_IHDR)) {
    IHDR *ihdr = (IHDR *)png->data;

    if (ihdr->colour_type == PNG_INDEXED) {
      while (png->id && (png->id != PNG_PLTE))
        png = png->next;

      if (png) {
        PLTE *plte = (PLTE *)png->data;
        pal = NewPalette(png->size / 3);
        CopyMem(plte->colors, pal->colors, png->size);
      }
    }
  }

  return pal;
}

#define ONSTACK(x) (&(x)), sizeof((x))

__regargs PngT *LoadPNG(CONST STRPTR filename, ULONG pngFlags) {
  PngT *root = NULL;
  FileT *fh;

  if ((fh = OpenFile(filename, IOF_BUFFERED))) {
    struct { LONG size, id; } chk;

    if (FileRead(fh, ONSTACK(chk)) && chk.size == PNG_ID0 && chk.id == PNG_ID1)
    {
      PngT *prev = NULL, *this;

      while (FileRead(fh, ONSTACK(chk))) {
        // Log("%4s : %ld\n", (STRPTR)&chk.id, chk.size);

        if (chk.id == PNG_IEND)
          break;

        if (chk.id == PNG_IHDR ||
            (chk.id == PNG_IDAT && !(pngFlags & PNG_SKIP_IDAT)) ||
            (chk.id == PNG_PLTE && !(pngFlags & PNG_SKIP_PLTE)) ||
            (chk.id == PNG_tRNS && !(pngFlags & PNG_SKIP_tRNS)))
        {
          this = MemAlloc(sizeof(PngT) + chk.size, MEMF_PUBLIC);
          this->size = chk.size;
          this->id = chk.id;
          FileRead(fh, this->data, chk.size);

          if (prev)
            prev->next = this;
          else
            root = this;

          prev = this;
        } else {
          FileSeek(fh, chk.size, SEEK_CUR);
        }

        /* skip CRC */
        FileSeek(fh, 4, SEEK_CUR);
      }
    } else {
      Print("'%s' : not a PNG file!\n", filename);
    }

    CloseFile(fh);
  } else {
    Log("File '%s' missing.\n", filename);
  }

  return root;
}

__regargs void DeletePNG(PngT *png) {
  if (png && (png->id == PNG_IHDR)) {
    while (png) {
      PngT *next = png->next;
      MemFree(png);
      png = next;
    }
  }
}

__regargs void PrintPNG(PngT *png) {
  if (png && (png->id == PNG_IHDR)) {
    while (png) {
      Log("%4s : %ld\n", (STRPTR)&png->id, png->size);
      MemDump(png->data, min(png->size, 128));
      png = png->next;
    }
  }
}
