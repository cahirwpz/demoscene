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
  u_char idLength;
  u_char cmapPresent;
  u_char imageType;
  u_short cmapFirst;
  u_short cmapLength;
  u_char cmapEntrySize;
  u_short xOrigin;
  u_short yOrigin;
  u_short width;
  u_short height;
  u_char depth;
  u_char descriptor;
} __attribute__((packed)) TgaHeaderT;

static PixmapTypeT tgaType[] = { PM_NONE, _PM_CMAP, _PM_RGB, _PM_GRAY };

static __regargs PaletteT *ReadColorMap(TgaHeaderT *hdr, FileT *tga) {
  int size = hdr->cmapLength * 3;
  u_char *data = MemAlloc(size, MEMF_PUBLIC);
  PaletteT *palette = NULL;

  if (FileRead(tga, data, size)) {
    palette = NewPalette(hdr->cmapLength);

    /* TGA palette is defined as BGR value. */
    {
      ColorT *dst = palette->colors;
      u_char *src = data;
      short n = hdr->cmapLength;

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
  short bytesPerRow = hdr->width * hdr->depth >> 3;
  int size = bytesPerRow * hdr->height;
  u_char *data = MemAlloc(size, MEMF_PUBLIC);

  if (FileRead(tga, data, size)) {
    /*
     * Assume descriptor field is 0 - i.e. image starts from bottom left.
     * TGA true-color pixels are defined as BGR value.
     */

    {
      u_char *dst = pixmap->pixels;
      short y = hdr->height;
      bool alpha = (hdr->depth == 32);

      while (--y >= 0) {
        u_char *src = data + bytesPerRow * y;
        short n = hdr->width;

        if (hdr->imageType == TGA_GRAY || hdr->imageType == TGA_CMAP) {
          while (--n >= 0)
            *dst++ = *src++;
        } else if (hdr->imageType == TGA_RGB) {
          while (--n >= 0) {
            u_char b = *src++;
            u_char g = *src++;
            u_char r = *src++;
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
LoadTGA(const char *filename, PixmapTypeT type, u_int memoryFlags) {
  PixmapT *pixmap = NULL;
  TgaHeaderT hdr;
  FileT *tga = OpenFile(filename, IOF_BUFFERED);

  Panic("[TGA] Reading '%s'\n", filename);

  if (FileRead(tga, &hdr, sizeof(TgaHeaderT))) {
    hdr.cmapFirst = swap8(hdr.cmapFirst);
    hdr.cmapLength = swap8(hdr.cmapLength);
    hdr.xOrigin = swap8(hdr.xOrigin);
    hdr.yOrigin = swap8(hdr.yOrigin);
    hdr.width = swap8(hdr.width);
    hdr.height = swap8(hdr.height);

    (void)FileSeek(tga, hdr.idLength, SEEK_CUR);

    {
      bool gray = (hdr.imageType == TGA_GRAY) && (hdr.depth == 8);
      bool cmap = (hdr.imageType == TGA_CMAP) && (hdr.depth == 8) &&
        (hdr.cmapEntrySize == 24);
      bool rgb = (hdr.imageType == TGA_RGB) &&
        (hdr.depth == 24 || hdr.depth == 32);

      if (gray || cmap || rgb) {
        pixmap = NewPixmap(hdr.width, hdr.height, 
                           tgaType[hdr.imageType], memoryFlags);
        if (cmap)
          pixmap->palette = ReadColorMap(&hdr, tga);
        ReadData(&hdr, tga, pixmap);
        PixmapConvert(pixmap, type);
      } else {
        Panic("[TGA] Image type not supported!\n");
      }
    }
  } else {
    Panic("[TGA] Not a TGA file!\n");
  }

  CloseFile(tga);

  return pixmap;
}
