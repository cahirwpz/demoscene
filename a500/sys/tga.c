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

static PixmapTypeT tgaType[] = { PM_NONE, _PM_CMAP, _PM_RGB, _PM_GRAY };

static __regargs PaletteT *ReadColorMap(TgaHeaderT *hdr, FileT *tga) {
  LONG size = hdr->cmapLength * 3;
  UBYTE *data = MemAlloc(size, MEMF_PUBLIC);
  PaletteT *palette = NULL;

  if (FileRead(tga, data, size)) {
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

  MemFree(data);
  return palette;
}

static __regargs void ReadData(TgaHeaderT *hdr, FileT *tga, PixmapT *pixmap) {
  WORD bytesPerRow = hdr->width * hdr->depth >> 3;
  LONG size = bytesPerRow * hdr->height;
  UBYTE *data = MemAlloc(size, MEMF_PUBLIC);

  if (FileRead(tga, data, size)) {
    /*
     * Assume descriptor field is 0 - i.e. image starts from bottom left.
     * TGA true-color pixels are defined as BGR value.
     */

    {
      UBYTE *dst = pixmap->pixels;
      WORD y = hdr->height;
      BOOL alpha = (hdr->depth == 32);

      while (--y >= 0) {
        UBYTE *src = data + bytesPerRow * y;
        WORD n = hdr->width;

        if (hdr->imageType == TGA_GRAY || hdr->imageType == TGA_CMAP) {
          while (--n >= 0)
            *dst++ = *src++;
        } else if (hdr->imageType == TGA_RGB) {
          while (--n >= 0) {
            UBYTE b = *src++;
            UBYTE g = *src++;
            UBYTE r = *src++;
            *dst++ = r;
            *dst++ = g;
            *dst++ = b;

            if (alpha)
              src++;
          }
        }
      }
    }
  }

  MemFree(data);
}

__regargs PixmapT *
LoadTGA(CONST STRPTR filename, PixmapTypeT type, ULONG memoryFlags) {
  PixmapT *pixmap = NULL;
  TgaHeaderT hdr;
  FileT *tga;

  if ((tga = OpenFile(filename, IOF_BUFFERED))) {
    if (FileRead(tga, &hdr, sizeof(TgaHeaderT))) {
      hdr.cmapFirst = swap8(hdr.cmapFirst);
      hdr.cmapLength = swap8(hdr.cmapLength);
      hdr.xOrigin = swap8(hdr.xOrigin);
      hdr.yOrigin = swap8(hdr.yOrigin);
      hdr.width = swap8(hdr.width);
      hdr.height = swap8(hdr.height);

      (void)FileSeek(tga, hdr.idLength, SEEK_CUR);

      {
        BOOL gray = (hdr.imageType == TGA_GRAY) && (hdr.depth == 8);
        BOOL cmap = (hdr.imageType == TGA_CMAP) && (hdr.depth == 8) &&
          (hdr.cmapEntrySize == 24);
        BOOL rgb = (hdr.imageType == TGA_RGB) &&
          (hdr.depth == 24 || hdr.depth == 32);

        if (gray || cmap || rgb) {
          pixmap = NewPixmap(hdr.width, hdr.height, 
                             tgaType[hdr.imageType], memoryFlags);
          if (cmap)
            pixmap->palette = ReadColorMap(&hdr, tga);
          ReadData(&hdr, tga, pixmap);
          PixmapConvert(pixmap, type);
        } else {
          Log("[TGA] Image type not supported!\n");
        }
      }
    }

    CloseFile(tga);
  } else {
    Log("File '%s' missing.\n", filename);
  }

  return pixmap;
}
