#include "memory.h"
#include "pixmap.h"
#include "io.h"
#include "tga.h"

#define TGA_NOIMAGE   0
#define TGA_CMAP      1
#define TGA_RGB       2
#define TGA_GRAY      3
#define TGA_RLE_CMAP  9
#define TGA_RLE_RGB   10
#define TGA_RLE_GRAY  11

typedef struct TgaHeader {
  UBYTE idLength;
  UBYTE cmapPresent;
  UBYTE imageType;
  UWORD cmapFirst;
  UWORD cmapLength;
  UBYTE cmapEntrySize;
  UWORD xOrigin;
  UWORD yOrigin;
  UWORD width;
  UWORD height;
  UBYTE depth;
  UBYTE descriptor;
} __attribute__((packed)) TgaHeaderT;

typedef struct TgaParser {
  LONG memoryFlags;
  PixmapTypeT type;
  FileT *file;
} TgaParserT;

static __regargs PaletteT *ReadColorMap(TgaHeaderT *hdr, TgaParserT *tga) {
  LONG size = hdr->cmapLength * 3;
  UBYTE *data = MemAllocAuto(size, MEMF_PUBLIC);
  PaletteT *palette = NULL;

  if (FileRead(tga->file, data, size)) {
    palette = NewPalette(hdr->cmapLength);

    /* TGA palette is defined as BGR value. */
    {
      ColorT *dst = palette->colors;
      UBYTE *src = data;
      WORD n = hdr->cmapLength;

      while (--n >= 0) {
        dst->b = *src++;
        dst->g = *src++;
        dst->r = *src++;
        dst++;
      }
    }
  }

  MemFreeAuto(data);
  return palette;
}

static __regargs void ConvertData8(PixmapT *pixmap, UBYTE *data) {
  UBYTE *dst = pixmap->pixels;
  WORD y = pixmap->height;
  WORD width = pixmap->width;

  while (--y >= 0) {
    UBYTE *src = data + width * y;
    WORD n = width;

    while (--n >= 0)
      *dst++ = *src++;
  }
}

static __regargs void ConvertData4(PixmapT *pixmap, UBYTE *data) {
  UBYTE *dst = pixmap->pixels;
  WORD y = pixmap->height;
  WORD width = pixmap->width;

  while (--y >= 0) {
    UBYTE *src = data + width * y;
    WORD n = width / 2;

    while (--n >= 0) {
      UBYTE a = *src++;
      UBYTE b = *src++;
      *dst++ = (a << 4) | b;
    }
  }
}

static __regargs void ConvertDataRGB(PixmapT *pixmap, UBYTE *data, WORD depth) {
  UWORD *dst = pixmap->pixels;
  WORD y = pixmap->height;
  WORD bytesPerRow = pixmap->width * depth / 8;
  BOOL alpha = (depth == 32);

  while (--y >= 0) {
    UBYTE *src = data + bytesPerRow * y;
    WORD n = pixmap->width;

    while (--n >= 0) {
      UBYTE b = *src++;
      UBYTE g = *src++;
      UBYTE r = *src++;
      UBYTE lo = (g & 0xf0) | ((b & 0xf0) >> 4);

      if (alpha) {
        UBYTE a = *src++;
        UBYTE hi = (~a & 0xf0) | ((r & 0xf0) >> 4);
        *dst++ = (hi << 8) | lo;
      } else {
        *dst++ = ((r & 0xf0) << 4) | lo;
      }
    }
  }
}

static __regargs PixmapT *ReadData(TgaHeaderT *hdr, TgaParserT *tga) {
  LONG size = hdr->width * hdr->height * (hdr->depth / 8);
  UBYTE *data = MemAllocAuto(size, MEMF_PUBLIC);
  PixmapT *pixmap = NULL;

  if (FileRead(tga->file, data, size)) {
    pixmap = NewPixmap(hdr->width, hdr->height, tga->type, tga->memoryFlags);

    /*
     * Assume descriptor field is 0 - i.e. image starts from bottom left.
     * TGA true-color pixels are defined as BGR value.
     */
    
    if (tga->type == PM_CMAP || tga->type == PM_GRAY) {
      ConvertData8(pixmap, data);
    } else if (tga->type == PM_CMAP4 || tga->type == PM_GRAY4) {
      ConvertData4(pixmap, data);
    } else if (tga->type == PM_RGB4) {
      ConvertDataRGB(pixmap, data, hdr->depth);
    }
  }

  MemFreeAuto(data);
  return pixmap;
}

__regargs PixmapT *
LoadTGA(CONST STRPTR filename, PixmapTypeT type, ULONG memoryFlags) {
  PixmapT *pixmap = NULL;
  TgaParserT parser;
  TgaHeaderT hdr;

  parser.type = type;
  parser.memoryFlags = memoryFlags;
  parser.file = OpenFile(filename, IOF_BUFFERED);

  if (parser.file) {
    if (FileRead(parser.file, &hdr, sizeof(TgaHeaderT))) {
      hdr.cmapFirst = swap8(hdr.cmapFirst);
      hdr.cmapLength = swap8(hdr.cmapLength);
      hdr.xOrigin = swap8(hdr.xOrigin);
      hdr.yOrigin = swap8(hdr.yOrigin);
      hdr.width = swap8(hdr.width);
      hdr.height = swap8(hdr.height);

      (void)FileSeek(parser.file, hdr.idLength, SEEK_SET);

      if (((type == PM_GRAY || type == PM_GRAY4) && 
           hdr.imageType == TGA_GRAY && hdr.depth == 8) ||
          (type == PM_RGB4 && hdr.imageType == TGA_RGB &&
           (hdr.depth == 24 || hdr.depth == 32)))
      {
        pixmap = ReadData(&hdr, &parser);
      } 
      else if ((type == PM_CMAP || type == PM_CMAP4) &&
               hdr.imageType == TGA_CMAP && hdr.depth == 8 &&
               hdr.cmapEntrySize == 24)
      {
        PaletteT *palette;
        
        if ((palette = ReadColorMap(&hdr, &parser))) {
          if ((pixmap = ReadData(&hdr, &parser)))
            pixmap->palette = palette;
          else
            DeletePalette(palette);
        }
      }
    }

    CloseFile(parser.file);
  } else {
    Log("File '%s' missing.\n", filename);
  }

  return pixmap;
}
